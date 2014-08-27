#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ocvEdgeDetectApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
    gl::Texture				mTexture;
	Capture					mCapture;
    cv::Mat input, gray, edges;
    bool isWorking;
};

void ocvEdgeDetectApp::setup()
{
    mCapture = Capture( 640, 480 );
	mCapture.start();
    isWorking = false;
}

void ocvEdgeDetectApp::mouseDown( MouseEvent event )
{
}

void ocvEdgeDetectApp::update()
{
    if( mCapture.checkNewFrame() ) {
        input = cv::Mat( toOcv( mCapture.getSurface() ) );
        cv::cvtColor( input, gray, CV_RGB2GRAY );
        cv::Canny( gray, edges, 10, 100, 3 );
        isWorking = true;
	}
}

void ocvEdgeDetectApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::clear();
	
	gl::color( Color( 1, 1, 1 ) );
    if (isWorking)
        gl::draw(fromOcv(edges));
}

CINDER_APP_NATIVE( ocvEdgeDetectApp, RendererGl )
