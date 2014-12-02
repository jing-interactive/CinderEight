#include "cinder/app/AppBasic.h"
#include "cinder/ArcBall.h"
#include "cinder/Camera.h"
#include "cinder/Surface.h"
#include "cinder/gl/Vbo.h"
#include "cinder/ImageIo.h"
#include "cinder/qtime/MovieWriter.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Device.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "cinder/params/Params.h"
#include "cinder/gl/GlslProg.h"
#include "Resources.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Perlin.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define INPUT_DEVICE "Scarlett 2i2 USB"

class ImageHFApp : public AppBasic {
public:
    void    setup();
    void    resize( );
    void    mouseDown( MouseEvent event );
    void    mouseDrag( MouseEvent event );
    void    keyDown( KeyEvent event );
    void 	prepareSettings( Settings* settings );
    void    draw();
    void	setVboMesh();
    void    update();
    void    setupAudio();
    
    CaptureRef				mCapture;
    gl::TextureRef			mTexture;
    qtime::MovieWriter	mMovieWriter;
    qtime::MovieGl		mMovie;
    
private:
    void setMovieWriter();
    void setMoviePlayer();
    
    CameraPersp			mCamera;
    MayaCamUI			mMayaCam;
    uint32_t            mWidth, mHeight;

    gl::VboMesh		mVboMesh;
    audio::InputDeviceNodeRef		mInputDeviceNode;
    audio::MonitorSpectralNodeRef	mMonitorSpectralNode;
    vector<float>					mMagSpectrum;
    uint16_t bandCount;
    Perlin				mPerlin;
    
    // width and height of our mesh
    static const int kWidth = 256;
    static const int kHeight = 256;
    
    // number of frequency bands of our spectrum
    static const int kBands = 1024;
    static const int kHistory = 128;
    uint32_t			mOffset;
    
    Channel32f			mChannelLeft;
    Channel32f			mChannelRight;
    gl::Texture::Format	mTextureFormat;
    gl::TextureRef		mTextureLeft;
    gl::TextureRef		mTextureRight;
    vector<gl::GlslProgRef>		mShader;
    int mShaderNum;
    float mFrameRate;
    bool  mAutomaticSwitch;
    ci::params::InterfaceGl		mParams;
};

void ImageHFApp::setup()
{
    
    mCapture = Capture::create( 640, 480 );// mWidth, mHeight );
    mCapture->start();

    mPerlin = Perlin( 4, 0 );
    mParams = params::InterfaceGl( "Params", Vec2i( 200, 300 ) );
//    mParams.addParam( "Frame rate",	&mFrameRate,"", true);
//    mParams.addParam( "Auto switch", &mAutomaticSwitch);
    mOffset = 0.0f;
    setupAudio();

    mShaderNum = 0;
    try {
        mShader.push_back(gl::GlslProg::create( loadResource( GLSL_VERT1 ), loadResource( GLSL_FRAG1 ) ));
        mShader.push_back(gl::GlslProg::create( loadResource( GLSL_VERT1 ), loadResource( GLSL_FRAG2 ) ));
        mShader.push_back(gl::GlslProg::create( loadResource( GLSL_VERT2 ), loadResource( GLSL_FRAG1 ) ));
        mShader.push_back(gl::GlslProg::create( loadResource( GLSL_VERT2 ), loadResource( GLSL_FRAG2 ) ));
    }
    catch( const std::exception& e ) {
        console() << e.what() << std::endl;
        quit();
        return;
    }

    // create channels from which we can construct our textures
    mChannelLeft = Channel32f(kBands, kHistory);
    mChannelRight = Channel32f(kBands, kHistory);
    memset(	mChannelLeft.getData(), 0, mChannelLeft.getRowBytes() * kHistory );
    memset(	mChannelRight.getData(), 0, mChannelRight.getRowBytes() * kHistory );
    
    // create texture format (wrap the y-axis, clamp the x-axis)
    mTextureFormat.setWrapS( GL_CLAMP );
    mTextureFormat.setWrapT( GL_REPEAT );
    mTextureFormat.setMinFilter( GL_LINEAR );
    mTextureFormat.setMagFilter( GL_LINEAR );
    
    //setMoviePlayer();
    try {
        if (!mMovie){
            mWidth = kWidth;
            mHeight = kHeight;
//            mCapture = Capture::create( 640, 480 );// mWidth, mHeight );
//            mCapture->start();
        }
    }
    catch( ... ) {
        console() << "Failed to initialize capture" << std::endl;
    }
    
    mCamera.setPerspective(50.0f, 1.0f, 1.0f, 10000.0f);
    mCamera.setEyePoint( Vec3f(-mWidth/2, mHeight/2, -mHeight/8) );
    //mCamera.setEyePoint( Vec3f(10239.3,7218.58,-7264.48));
    mCamera.setCenterOfInterestPoint( Vec3f(mWidth*0.5f, -mHeight*0.5f, mWidth*0.5f) );

    setVboMesh();
    //setMovieWriter();
}

void ImageHFApp::setMoviePlayer() {
    string moviePath = getOpenFilePath().string();
    if( ! moviePath.empty() ){
        try {
            // load up the movie, set it to loop, and begin playing
            mMovie = qtime::MovieGl( moviePath );
            mWidth = mMovie.getWidth();
            mHeight = mMovie.getHeight();
            mMovie.setLoop();
            mMovie.play();
        }
        catch( ... ) {
            console() << "Unable to load the movie." << std::endl;
            mMovie.reset();
        }
    }
}
void ImageHFApp::setMovieWriter(){
    string path = getSaveFilePath().string();
    if( path.empty() ) return; // user cancelled save
    qtime::MovieWriter::Format format;
    if( qtime::MovieWriter::getUserCompressionSettings( &format) ) {
        mMovieWriter = qtime::MovieWriter( path, getWindowWidth(), getWindowHeight(), format );
    }
}

void ImageHFApp::update() {
    
    mFrameRate = getAverageFps();
    if( mCapture && mCapture->checkNewFrame() ) {
        mTexture = gl::Texture::create( mCapture->getSurface() );
    } else if ( mMovie ){
        //mTexture = mMovie.getTexture();
    }
    
    return;
    
    mMagSpectrum = mMonitorSpectralNode->getMagSpectrum();
    
    // get spectrum for left and right channels and copy it into our channels
    float* pDataLeft = mChannelLeft.getData() + kBands * mOffset;
    float* pDataRight = mChannelRight.getData() + kBands * mOffset;
    
    std::reverse_copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataLeft);
    std::copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataRight);
    
    // increment texture offset
    mOffset = (mOffset+1) % kHistory;
    {
        float t = float( getElapsedSeconds() );
        float x = 0.5f * math<float>::cos( t * 0.07f );
        float y = 0.5f * math<float>::sin( t * 0.09f );//0.1f - 0.2f * math<float>::sin( t * 0.09f );
        float z = 0.05f * math<float>::sin( t * 0.05f ) - 0.15f;
        
        Vec3f eye = Vec3f(kWidth * x, kHeight * y*0.1f, kHeight * z);
        
        x = 1.0f - x;
        y = -0.5f;
        z = 0.6f + 0.2f *  math<float>::sin( t * 0.12f );
        
        Vec3f interest = Vec3f(kWidth * x, kHeight * y*0.1f, kHeight * z);
        //cout<<interest<< " "<< (eye.lerp(0.995f, mCamera.getEyePoint()))<<endl;
        
        // gradually move to eye position and center of interest
        float correction = 1.0 - 0.1*mMonitorSpectralNode->getVolume();
        mCamera.setEyePoint( eye.lerp(0.995f*correction, mCamera.getEyePoint()) );
        mCamera.setCenterOfInterestPoint( interest.lerp(0.990f*correction, mCamera.getCenterOfInterestPoint()) );

        if (mAutomaticSwitch &&  (mMonitorSpectralNode->getVolume() < 0.001f || mMonitorSpectralNode->getVolume() > 0.5f)){
            mShaderNum = mShaderNum == mShader.size() - 1 ? 0 : mShaderNum + 1;
        }
    }
}

void ImageHFApp::setVboMesh()
{
    std::vector<Colorf>     colors;
    std::vector<Vec2f>      coords;
    std::vector<uint32_t>	indices;
    std::vector<Vec3f>      mVertices;
    
    for(size_t h=0;h<kHeight;++h)
    {
        for(size_t w=0;w<kWidth;++w)
        {
            // add polygon indices
            if(h < kHeight-1 && w < kWidth-1)
            {
                size_t offset = mVertices.size();
                
                indices.push_back(offset);
                indices.push_back(offset+kWidth);
                indices.push_back(offset+kWidth+1);
                indices.push_back(offset);
                indices.push_back(offset+kWidth+1);
                indices.push_back(offset+1);
            }
            
            // add vertex
            float value = 80.0f;//* mPerlin.fBm(Vec3f(float(h), float(w), 0.f) * 0.005f);
            mVertices.push_back( Vec3f(float(w), value, float(h)) );
            
            // add texture coordinates
            // note: we only want to draw the lower part of the frequency bands,
            //  so we scale the coordinates a bit
            const float part = 0.5f;
            float s = w / float(kWidth-1);
            float t = h / float(kHeight-1);
            coords.push_back( Vec2f(part - part * s, t) );
            
            // add vertex colors
            colors.push_back( h % 2 == 0 || true ? Color(CM_HSV, s, 1.0f, 1.0f) : Color(CM_RGB, s, s, s) );
        }
    }
    
    gl::VboMesh::Layout layout;
    layout.setStaticPositions();
    //    layout.setDynamicPositions();
    layout.setStaticColorsRGB();
    layout.setStaticIndices();
    layout.setStaticTexCoords2d();
    
    mVboMesh = gl::VboMesh(mVertices.size(), indices.size(), layout, GL_TRIANGLES);
    mVboMesh.bufferIndices(indices);
    return;
    mVboMesh.bufferTexCoords2d(0, coords);
    mVboMesh.bufferPositions(mVertices);

    mVboMesh.bufferColorsRGB(colors);
}

void ImageHFApp::resize( )
{
    mCamera.setAspectRatio( getWindowAspectRatio() );
}

void ImageHFApp::mouseDown( MouseEvent event )
{
    mMayaCam.setCurrentCam(mCamera);
    mMayaCam.mouseDown( event.getPos() );
}

void ImageHFApp::mouseDrag( MouseEvent event )
{
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
    mCamera = mMayaCam.getCamera();
}

void ImageHFApp::keyDown( KeyEvent event )
{
    switch( event.getChar() ) {
        case '+':
            break;
        case '-':
            break;
    }

    if( mMovie ) {
        if( event.getCode() == KeyEvent::KEY_LEFT ) {
            mMovie.stepBackward();
        }
        if( event.getCode() == KeyEvent::KEY_RIGHT ) {
            mMovie.stepForward();
        }
        else if( event.getChar() == 'm' ) {
            // jump to the middle frame
            mMovie.seekToTime( mMovie.getDuration() / 2 );
        }
        else if( event.getChar() == ' ' ) {
            if( mMovie.isPlaying() )
                mMovie.stop();
            else
                mMovie.play();
        }
    }
}

void ImageHFApp::draw()
{
    gl::clear( Color( 0.0f, 0.0f, 0.0f ) );
    gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
    
//    if( mTexture ) {
//        glPushMatrix();
//        gl::draw( mTexture );
//        glPopMatrix();
//    }
//    
//    return;
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    gl::pushMatrices();
    gl::setMatrices(mCamera);
    {
        if( mVboMesh && mTexture){
            // bind shader
            mShader[mShaderNum]->bind();
            float offSt = mOffset / float(kHistory);
            mShader[mShaderNum]->uniform("uTexOffset", offSt);
            mShader[mShaderNum]->uniform("uLeftTex", 0);
            mShader[mShaderNum]->uniform("uRightTex", 1);
            mShader[mShaderNum]->uniform("uVideoTex",2);
            mShader[mShaderNum]->uniform("resolution", 0.5f*(float)kWidth);

            // create textures from our channels and bind them
            mTextureLeft = gl::Texture::create(mChannelLeft, mTextureFormat);
            mTextureRight = gl::Texture::create(mChannelRight, mTextureFormat);
            
            mTextureLeft->enableAndBind();
            mTextureRight->bind(1);
            mTexture->bind(2);
            
            gl::enableAdditiveBlending();

            gl::draw( mVboMesh );
            
            gl::disableAlphaBlending();

            mTexture->unbind();
            mTextureLeft->unbind();
            mTextureRight->unbind();
        }
    }
    gl::popMatrices();
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    // add this frame to our movie
//    if( mMovieWriter )
//        mMovieWriter.addFrame( copyWindowSurface() );
    
    //mParams.draw();
}

inline void ImageHFApp::setupAudio() {
    auto ctx = audio::Context::master();
    std::cout << "Devices available: " << endl;
    for( const auto &dev : audio::Device::getInputDevices() ) {
        std::cout<<dev->getName() <<endl;
    }
    
    std::vector<audio::DeviceRef> devices = audio::Device::getInputDevices();
    const auto dev = audio::Device::findDeviceByName(INPUT_DEVICE);
    if (!dev){
        cout<<"Could not find " << INPUT_DEVICE << endl;
        mInputDeviceNode = ctx->createInputDeviceNode();
        cout<<"Using default input"<<endl;
    } else {
        mInputDeviceNode = ctx->createInputDeviceNode(dev);
    }
    
    // By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
    // an increase in resolution of the resulting spectrum data.
    auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize( kBands ).windowSize( kBands / 2 );
    mMonitorSpectralNode = ctx->makeNode( new audio::MonitorSpectralNode( monitorFormat ) );
    
    mInputDeviceNode >> mMonitorSpectralNode;
    
    // InputDeviceNode (and all InputNode subclasses) need to be enabled()'s to process audio. So does the Context:
    mInputDeviceNode->enable();
    ctx->enable();
    
    getWindow()->setTitle( mInputDeviceNode->getDevice()->getName() );
}

inline void ImageHFApp::prepareSettings(Settings* settings) {
    settings->setWindowSize(1052, 760);
}

CINDER_APP_BASIC( ImageHFApp, RendererGl );

