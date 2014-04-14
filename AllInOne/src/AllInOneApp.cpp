#include "cinder/app/AppNative.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "cinder/Text.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"
#include "cinder/qtime/QuickTime.h"

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

static const int WIDTH = 640, HEIGHT = 480;
static const float ONE_OVER_255 = 1.0/255.;
static const int AVERAGE_TYPE = 1;
static const int SCREEN_TYPE = 2;

#define toDigit(c) (c-'0')

class AllInOneApp : public AppNative {
public:
	void setup();
	void keyDown( KeyEvent event );
	void update();
	void draw();
    void fileDrop( ci::app::FileDropEvent event );
	
private:
	CaptureRef		mCapture;
	gl::TextureRef	mTexture;
	gl::TextureRef	mNameTexture;
	Surface		    mSurface, mCumulativeSurface, mPrevSurface, mWhiteSurface;
    size_t          frameNum;
    Color           averageColor;
    int type;
    qtime::MovieSurface mMovie;
    void computeAverage();
    void computeScreen();
};

void AllInOneApp::setup()
{
	// list out the devices
	vector<Capture::DeviceRef> devices( Capture::getDevices() );
	for( vector<Capture::DeviceRef>::const_iterator deviceIt = devices.begin(); deviceIt != devices.end(); ++deviceIt ) {
		Capture::DeviceRef device = *deviceIt;
		console() << "Found Device " << device->getName() << " ID: " << device->getUniqueId() << std::endl;
        try {
            mCapture = Capture::create( WIDTH, HEIGHT );
            mCapture->start();
            
            TextLayout layout;
            layout.setFont( Font( "Arial", 24 ) );
            layout.setColor( Color( 1, 1, 1 ) );
            layout.addLine( device->getName() );
            mNameTexture = gl::Texture::create( layout.render( true ) ) ;
        }
        catch( ... ) {
            console() << "Failed to initialize capture" << std::endl;
        }
	}
    frameNum = 0;
    type = AVERAGE_TYPE;

    getWindow()->setTitle("All In One by eight_io");
}

void AllInOneApp::fileDrop( ci::app::FileDropEvent event ){

    fs::path moviePath = event.getFile(0);
	if( moviePath.empty() ) return;

    try {
		// load up the movie, set it to loop, and begin playing
        mMovie = qtime::MovieSurface( moviePath );
		mMovie.setLoop(true);
        mMovie.play();
        mCapture -> stop();
        frameNum = 0;
	}
	catch( ... ) {
		console() << "Unable to load the movie." << std::endl;
		mMovie.reset();
		mTexture.reset();
	}
}

void AllInOneApp::keyDown( KeyEvent event )
{
    switch( event.getChar() ) {
        case 'f': setFullScreen( ! isFullScreen() ); break;
        case ' ':
            mCapture->isCapturing() ? mCapture->stop() : mCapture->start();
            break;
		case 's':
            {
                boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
                std::stringstream timestamp;
                timestamp << now;
                writeImage( getHomeDirectory() / ("Desktop/AllInOne-"+timestamp.str()+".png"), mCumulativeSurface );
            }
            break;
        case 'r':
            frameNum = 0;
        default:
            if (isdigit(event.getChar())){
                int newType = toDigit(event.getChar());
                if (newType != type && (newType >=AVERAGE_TYPE && newType <= SCREEN_TYPE)){
                    type = newType;
                    frameNum = 0;
                }
            }
            break;
	}
}

void AllInOneApp::computeAverage(){
    float oneOverFrameNum = 1./(float)frameNum;
    auto iter = mSurface.getIter( );
    auto mCumulativeIter = mCumulativeSurface.getIter();
    while( iter.line() && mCumulativeIter.line()) {
        while( iter.pixel() && mCumulativeIter.pixel()) {
            //avg(i) = (i-1)/i*avg(i-1) + x(i)/i;
            mCumulativeIter.r() = ((frameNum-1) * mCumulativeIter.r() + iter.r()) * oneOverFrameNum;
            mCumulativeIter.g() = ((frameNum-1) * mCumulativeIter.g() + iter.g()) * oneOverFrameNum;
            mCumulativeIter.b() = ((frameNum-1) * mCumulativeIter.b() + iter.b()) * oneOverFrameNum;
        }
    }
    averageColor = mCumulativeSurface.areaAverage(mCumulativeSurface.getBounds());
    mTexture = gl::Texture::create(mCumulativeSurface);
}

void AllInOneApp::computeScreen(){
    
    //apply screen blending to previous surface
    if (frameNum > 10 && false) {
        mSurface = Surface(mSurface.getWidth(), mSurface.getHeight(),false);
        auto it = mSurface.getIter();
        while (it.line()) {
            while(it.pixel()){
                it.r() = 255;
                it.g() = 255;
                it.b() = 255;
            }
        }
    }
    auto iter = mSurface.getIter( );
    auto prevIter = mPrevSurface.getIter();
    while( iter.line() && prevIter.line()) {
        while( iter.pixel() && prevIter.pixel()) {
            //result = one - (one - a) * (one - b);
            prevIter.r() = 255 - (255 - prevIter.r()) * (255 - iter.r()) * ONE_OVER_255;
            prevIter.g() = 255 - (255 - prevIter.g()) * (255 - iter.g()) * ONE_OVER_255;
            prevIter.b() = 255 - (255 - prevIter.b()) * (255 - iter.b()) * ONE_OVER_255;
        }
    }
    
    //accumulate partial screen blending
    float oneOverFrameNum = 1./(float)frameNum;
    prevIter = mPrevSurface.getIter( );
    auto mCumulativeIter = mCumulativeSurface.getIter();
    while( prevIter.line() && mCumulativeIter.line()) {
        while( prevIter.pixel() && mCumulativeIter.pixel()) {
            //avg(i) = (i-1)/i*avg(i-1) + x(i)/i;
            int prv = prevIter.r();
            int cml = mCumulativeIter.r();
            mCumulativeIter.r() = ((frameNum-1) * mCumulativeIter.r() + prevIter.r()) * oneOverFrameNum;
            cml = mCumulativeIter.r();
            mCumulativeIter.g() = ((frameNum-1) * mCumulativeIter.g() + prevIter.g()) * oneOverFrameNum;
            mCumulativeIter.b() = ((frameNum-1) * mCumulativeIter.b() + prevIter.b()) * oneOverFrameNum;
        }
    }

    mTexture = gl::Texture::create(mCumulativeSurface);
    averageColor = mCumulativeSurface.areaAverage(mCumulativeSurface.getBounds());

    //retain current surface for next iteration
    mPrevSurface.copyFrom(mSurface, mSurface.getBounds());
}

void AllInOneApp::update()
{
    bool isUpdated = false;
	if ( mCapture->checkNewFrame() ) {
        mSurface = mCapture->getSurface();
        isUpdated = true;
    }
    if (!isUpdated && mMovie && mMovie.checkNewFrame()){
        mSurface = mMovie.getSurface();
        isUpdated = true;
    }
    
    if (!isUpdated) return;

        if (frameNum > 0){
            switch (type) {
                case AVERAGE_TYPE:
                    computeAverage();
                    break;
                case SCREEN_TYPE:
                    computeScreen();
                    break;
                default:
                    break;
            } 
        } else {

            Area area = mSurface.getBounds();
            mCumulativeSurface = Surface( area.getWidth(), area.getHeight(), false );
            
            switch (type) {
                case AVERAGE_TYPE:
                    mCumulativeSurface.copyFrom(mSurface, area);
                    break;
                case SCREEN_TYPE:
                {
                    mPrevSurface = Surface(area.getWidth(), area.getHeight(),false);
                    mPrevSurface.copyFrom(mSurface, mSurface.getBounds());
                    
                    auto prevIter = mPrevSurface.getIter();
                    while( prevIter.line()) {
                        while( prevIter.pixel() ) {
                            prevIter.r() = 255 - prevIter.r();
                            prevIter.g() = 255 - prevIter.g();
                            prevIter.b() = 255 - prevIter.b();
                        }
                    }
                    mCumulativeSurface.copyFrom(mPrevSurface, area);
                    break;
                }
                default:
                    break;
            }
            mTexture = gl::Texture::create (mSurface);
        }
        frameNum++;
}

void AllInOneApp::draw()
{
	//gl::enableAlphaBlending();
	gl::clear( Color::black() );

    // draw the latest frame
    gl::color( Color::white() );
    if( mTexture)
        gl::draw( mTexture, Rectf( 0, 0, getWindowWidth(), getWindowHeight()) );
    
    gl::drawString("color "+ toString((float) averageColor.length())+ " "+ toString(getFrameRate())+" FPS", Vec2f(5.0f, 5.0f));
    
}

CINDER_APP_NATIVE( AllInOneApp, RendererGl )
