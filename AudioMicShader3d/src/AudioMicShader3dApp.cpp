/*
 Copyright (c) 2014, Paul Houx - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Camera.h"
#include "cinder/Channel.h"
#include "cinder/ImageIo.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Rand.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Device.h"
#include "cinderSyphon.h"
#include "cinder/gl/Light.h"
#include "Resources.h"
#include "cinder/Perlin.h"
#include "cinder/params/Params.h"

#define INPUT_DEVICE "Scarlett 2i2 USB"

using namespace ci;
using namespace ci::app;
using namespace std;

class AudioVisualizerApp : public AppNative {
public:
    void prepareSettings( Settings* settings );
    
    void setup();
    void shutdown();
    void update();
    void draw();
    
    void mouseDown( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void mouseUp( MouseEvent event );
    void keyDown( KeyEvent event );
    void mouseWheel( MouseEvent event );
    void resize();
    
private:
    // width and height of our mesh
    static const int kWidth = 256;
    static const int kHeight = 256;
    
    // number of frequency bands of our spectrum
    static const int kBands = 1024;
    static const int kHistory = 128;
    
    Channel32f			mChannelLeft;
    Channel32f			mChannelRight;
    CameraPersp			mCamera;
    MayaCamUI			mMayaCam;
    gl::GlslProg		mShader;
    gl::TextureRef			mTextureLeft;
    gl::TextureRef		mTextureRight;
    gl::Texture::Format	mTextureFormat;
    gl::VboMesh			mMesh;

    uint32_t			mOffset;
    
    bool				mIsMouseDown;
    bool				mIsAudioPlaying;
    double				mMouseUpTime;
    double				mMouseUpDelay;
    
    vector<string>		mAudioExtensions;
    
    audio::InputDeviceNodeRef		mInputDeviceNode;
    audio::MonitorSpectralNodeRef	mMonitorSpectralNode;
    vector<float>					mMagSpectrum;
    Perlin				mPerlin;
    uint32              mPerlinMove;
    std::vector<Vec3f>      mVertices;
    
	syphonServer mTextureSyphon;
    
    // Lighting
	ci::gl::Light				*mLight;
	bool						mLightEnabled;
    float mFrameRate;
    ci::params::InterfaceGl		mParams;

};

void AudioVisualizerApp::prepareSettings(Settings* settings)
{
    settings->setFullScreen(false);
    settings->setWindowSize(1280, 720);
}

void AudioVisualizerApp::mouseWheel( MouseEvent event )
{
	// Zoom in/out with mouse wheel
	Vec3f eye = mCamera.getEyePoint();
	eye.z += event.getWheelIncrement();
	mCamera.setEyePoint( eye );
}

void AudioVisualizerApp::setup()
{
    
    mPerlin = Perlin( 4, 0 );
    mPerlinMove = 0;
    mFrameRate			= 0.0f;
    mParams = params::InterfaceGl( "Params", Vec2i( 200, 100 ) );
	mParams.addParam( "Frame rate",	&mFrameRate,"", true);

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
    auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize( 2048 ).windowSize( 1024 );
    mMonitorSpectralNode = ctx->makeNode( new audio::MonitorSpectralNode( monitorFormat ) );
    
    mInputDeviceNode >> mMonitorSpectralNode;
    
    // InputDeviceNode (and all InputNode subclasses) need to be enabled()'s to process audio. So does the Context:
    mInputDeviceNode->enable();
    ctx->enable();
    
    getWindow()->setTitle( mInputDeviceNode->getDevice()->getName() );
    //////
    
    // initialize signals
    
    mIsAudioPlaying = false;
    
    // setup camera
    mCamera.setPerspective(50.0f, 1.0f, 1.0f, 10000.0f);
    mCamera.setEyePoint( Vec3f(-kWidth/2, kHeight/2, -kWidth/8) );
    //mCamera.setEyePoint( Vec3f(10239.3,7218.58,-7264.48));
    mCamera.setCenterOfInterestPoint( Vec3f(kWidth/4, -kHeight/8, kWidth/4) );
    
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

    try {
        mShader = gl::GlslProg( loadResource( GLSL_VERT ), loadResource( GLSL_FRAG ) );
    }
    catch( const std::exception& e ) {
        console() << e.what() << std::endl;
        quit();
        return;
    }
    
    // create static mesh (all animation is done in the vertex shader)

    std::vector<Colorf>     colors;
    std::vector<Vec2f>      coords;
    std::vector<uint32_t>	indices;
    
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
            float value = 20.0f*mPerlin.fBm(Vec3f(float(h), float(w), 0.f)* 0.005f);
            mVertices.push_back( Vec3f(float(w), value, float(h)) );
            
            // add texture coordinates
            // note: we only want to draw the lower part of the frequency bands,
            //  so we scale the coordinates a bit
            const float part = 0.5f;
            float s = w / float(kWidth-1);
            float t = h / float(kHeight-1);
            coords.push_back( Vec2f(part - part * s, t) );
            
            // add vertex colors
            colors.push_back( Color(CM_HSV, s, 1.0f, 1.0f) );
        }
    }
    
    gl::VboMesh::Layout layout;
    layout.setStaticPositions();
    layout.setStaticColorsRGB();
    layout.setStaticIndices();
    layout.setStaticTexCoords2d();
    
    mMesh = gl::VboMesh(mVertices.size(), indices.size(), layout, GL_TRIANGLES);
    mMesh.bufferPositions(mVertices);
    mMesh.bufferColorsRGB(colors);
    mMesh.bufferIndices(indices);
    mMesh.bufferTexCoords2d(0, coords);
    
    vector<Vec3f> normals;
    
    // Iterate through again to set normals
//    for ( int32_t y = 0; y < mResolution.y - 1; y++ ) {
//        for ( int32_t x = 0; x < mResolution.x - 1; x++ ) {
//            Vec3f vert0 = positions[ indices[ ( x + mResolution.x * y ) * 6 ] ];
//            Vec3f vert1 = positions[ indices[ ( ( x + 1 ) + mResolution.x * y ) * 6 ] ];
//            Vec3f vert2 = positions[ indices[ ( ( x + 1 ) + mResolution.x * ( y + 1 ) ) * 6 ] ];
//            normals[ x + mResolution.x * y ] = Vec3f( ( vert1 - vert0 ).cross( vert1 - vert2 ).normalized() );
//        }
//    }

    mIsMouseDown = false;
    mMouseUpDelay = 5.0;
    mMouseUpTime = getElapsedSeconds() - mMouseUpDelay;
    
    // the texture offset has two purposes:
    //  1) it tells us where to upload the next spectrum data
    //  2) we use it to offset the texture coordinates in the shader for the scrolling effect
    mOffset = 0;
    
    mTextureSyphon.setName("Mic3d");
    
    // Set up OpenGL to work with default lighting
//	glShadeModel( GL_SMOOTH );
//	gl::enable( GL_POLYGON_SMOOTH );
//	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
//	gl::enable( GL_NORMALIZE );
//	gl::enableAlphaBlending();
//	gl::enableDepthRead();
//	gl::enableDepthWrite();
//    
//    
//    // Set up the light
//	mLight = new gl::Light( gl::Light::DIRECTIONAL, 0 );
//	mLight->setAmbient( ColorAf::white() );
//	mLight->setDiffuse( ColorAf::white() );
//	mLight->setDirection( Vec3f::one() );
//	mLight->setPosition( Vec3f::one() * -1.0f );
//	mLight->setSpecular( ColorAf::white() );
//	mLight->enable();
    mLightEnabled = false;
    
    //setFrameRate(30.0f);
    
    
    //fog
    
//    GLfloat density = 0.01;
//    
//    GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1.0};
//    
//    glEnable (GL_DEPTH_TEST); //enable the depth testing
//    
//    glEnable (GL_FOG);
//    
//    glFogi (GL_FOG_MODE, GL_EXP2);
//    
//    glFogfv (GL_FOG_COLOR, fogColor);
//    
//    glFogf (GL_FOG_DENSITY, density);
//    
//    glHint (GL_FOG_HINT, GL_NICEST);
}

void AudioVisualizerApp::shutdown()
{

}

void AudioVisualizerApp::update()
{
    mFrameRate = getAverageFps();

    mMagSpectrum = mMonitorSpectralNode->getMagSpectrum();
    
    // get spectrum for left and right channels and copy it into our channels
    
    float* pDataLeft = mChannelLeft.getData() + kBands * mOffset;
    float* pDataRight = mChannelRight.getData() + kBands * mOffset;

    std::copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataLeft);
    std::copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataRight);

    // increment texture offset
    mOffset = (mOffset+1) % kHistory;

    // clear the spectrum for this row to avoid old data from showing up
    pDataLeft = mChannelLeft.getData() + kBands * mOffset;
    pDataRight = mChannelRight.getData() + kBands * mOffset;
    
    memset( pDataLeft, 0, kBands * sizeof(float) );
    memset( pDataRight, 0, kBands * sizeof(float) );

    // animate camera if mouse has not been down for more than 30 seconds

    if(true || !mIsMouseDown && (getElapsedSeconds() - mMouseUpTime) > mMouseUpDelay)
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
        
        mCamera.setEyePoint( eye.lerp(0.995f, mCamera.getEyePoint()) );
        mCamera.setCenterOfInterestPoint( interest.lerp(0.990f, mCamera.getCenterOfInterestPoint()) );
    }
    
    // Update light on every frame
	//mLight->update( mCamera );
    mPerlinMove++;
    mVertices.clear();
    double clear = getElapsedSeconds();

    for(size_t h=0;h<kHeight;++h)
    {
        for(size_t w=0;w<kWidth;++w)
        {
            float value = 40.0f*mPerlin.fBm(Vec3f(float(h+ mPerlinMove), float(w), 0.f)* 0.005f);
            mVertices.push_back( Vec3f(float(w), value, float(h)) );
        }
    }
    double updt = getElapsedSeconds();

    mMesh.bufferPositions(mVertices);

//    gl::VboMesh::VertexIter iter = mIcosahedron.mapVertexBuffer();
//    int x = 0;
//    int y = 0;
//    for( int idx = 0; idx < mIcosahedron.getNumVertices(); ++idx ) {
//        float value = 2.0f*mPerlin.fBm(Vec3f(float(x+mPerlinMove), float(y), 0.f)* 0.005f);
//        Vec3f position( (float)x - halfWidth, value, (float)y - halfHeight );
//        
//        iter.setPosition( position * scale + offset  );
//        ++iter;
//        ++x;
//        if ( x == (int)mResolution.x){
//            x = 0;
//            y++;
//        }
//        
//    }

}


void AudioVisualizerApp::draw()
{
    gl::clear();
    if ( mLightEnabled ) {
        gl::enable( GL_LIGHTING );
    }
//    gl::enableDepthRead();
//    gl::enableDepthWrite();
    // use camera
    gl::pushMatrices();
    gl::setMatrices(mCamera);
    {

        // bind shader
        mShader.bind();
        float offSt = mOffset / float(kHistory);
        mShader.uniform("uTexOffset", offSt);
        mShader.uniform("uLeftTex", 0);
        mShader.uniform("uRightTex", 1);
        //mShader.uniform("elTime", (float) getElapsedFrames());
        
        // create textures from our channels and bind them
        mTextureLeft = gl::Texture::create(mChannelLeft, mTextureFormat);
        mTextureRight = gl::Texture::create(mChannelRight, mTextureFormat);
        
        mTextureLeft->enableAndBind();
        mTextureRight->bind(1);
        
        // draw mesh using additive blending
        gl::enableAdditiveBlending();

        gl::color( Color(1, 1, 1) );
        gl::draw( mMesh );

        gl::disableAlphaBlending();

        // unbind textures and shader
        mTextureRight->unbind();
        mTextureLeft->unbind();
        mShader.unbind();
    }

    gl::popMatrices();
//    gl::disableDepthRead();
//    gl::disableDepthWrite();
    if ( mLightEnabled ) {
		gl::disable( GL_LIGHTING );
	}
    mTextureSyphon.publishScreen();
    
    mParams.draw();
    
}

void AudioVisualizerApp::mouseDown( MouseEvent event )
{
    // handle mouse down
    mIsMouseDown = true;
    
    mMayaCam.setCurrentCam(mCamera);
    mMayaCam.mouseDown( event.getPos() );
    //cout<<mMayaCam.getCamera().getEyePoint()<<endl;
}

void AudioVisualizerApp::mouseDrag( MouseEvent event )
{
    // handle mouse drag
    mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
    mCamera = mMayaCam.getCamera();
    //cout<<"D "<<mMayaCam.getCamera().getEyePoint()<<endl;
}

void AudioVisualizerApp::mouseUp( MouseEvent event )
{
    // handle mouse up
    mMouseUpTime = getElapsedSeconds();
    mIsMouseDown = false;
}

void AudioVisualizerApp::keyDown( KeyEvent event )
{
    // handle key down
    switch( event.getCode() )
    {
        case KeyEvent::KEY_ESCAPE:
            quit();
            break;
        case KeyEvent::KEY_F4:
            if( event.isAltDown() )
                quit();
            break;
        case KeyEvent::KEY_LEFT:
            
            break;
        case KeyEvent::KEY_RIGHT:
            break;
        case KeyEvent::KEY_f:
            setFullScreen( !isFullScreen() );
            break;
        case KeyEvent::KEY_o:
            break;
        case KeyEvent::KEY_p:
            break;
        case KeyEvent::KEY_s:
            
            break;
    }
}

void AudioVisualizerApp::resize()
{
    // handle resize
    mCamera.setAspectRatio( getWindowAspectRatio() );
}

CINDER_APP_NATIVE( AudioVisualizerApp, RendererGl )