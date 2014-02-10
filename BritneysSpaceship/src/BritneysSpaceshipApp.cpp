#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/audio/Input.h"
#include "cinder/audio/FftProcessor.h"
#include "Resources.h"

using namespace ci;
using namespace ci::app;

class BritneysSpaceshipApp : public AppBasic {
public:
	void setup();
	void keyDown( KeyEvent event );
	
	void update();
	void draw();
	
	gl::TextureRef	soundTexture, palletteTexture;
	gl::GlslProgRef	mShader;
    
    audio::Input mInput;
    std::shared_ptr<float> mFftDataRef;
	audio::PcmBuffer32fRef mPcmBuffer;
};


void BritneysSpaceshipApp::setup()
{
	try {
		palletteTexture = gl::Texture::create( loadImage( loadResource( RES_IMAGE_JPG ) ) );
	}
	catch( ... ) {
		std::cout << "unable to load the texture file!" << std::endl;
	}
	
	try {
		mShader = gl::GlslProg::create( loadResource( RES_PASSTHRU_VERT ), loadResource( RES_BLUR_FRAG ) );
	}
	catch( gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
	catch( ... ) {
		std::cout << "Unable to load shader" << std::endl;
	}
    
	const std::vector<audio::InputDeviceRef>& devices = audio::Input::getDevices();
	for( std::vector<audio::InputDeviceRef>::const_iterator iter = devices.begin(); iter != devices.end(); ++iter ) {
		console() << (*iter)->getName() << std::endl;
	}
    
	mInput = audio::Input();
    
	mInput.start();
    
}

void BritneysSpaceshipApp::keyDown( KeyEvent event )
{
	if( event.getCode() == app::KeyEvent::KEY_f ) {
		setFullScreen( ! isFullScreen() );
	}
}

void BritneysSpaceshipApp::update()
{
    
    mPcmBuffer = mInput.getPcmBuffer();
	if( ! mPcmBuffer ) {
		return;
	}
    
    /*
     shadertoy: The FFT signal, which is 512 pixels/frequencies long, gets normalized to 0..1 and mapped to 0..255.
     The wave form, which is also 512 pixels/sampled long, gets renormalized too from -16387..16384 to 0..1. (and then to 0..255???)
     FFT goes in the first row, waveform in the second row. So this is a 512x2 gray scale 8 bit texture.
     */
    
	uint16_t bandCount = 512;
	mFftDataRef = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ), bandCount );
    
    float * fftBuffer = mFftDataRef.get();
    if (!fftBuffer) return;
    
	
    //create a sound texture as 512x2
	unsigned char signal[1024];
    
    //the first row is the spectrum (shadertoy)
    float max = 0.;
    for(int i=0;i<512;++i){
		if (fftBuffer[i] > max) max = fftBuffer[i];
    }
    
    float ht = 255.0/max;
	for(int i=0;i<512;++i){
		signal[i] = (unsigned char) (fftBuffer[i] * ht);
    }
    
    //waveform
    uint32_t bufferSamples = mPcmBuffer->getSampleCount();
	audio::Buffer32fRef leftBuffer = mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT );
    
	int endIdx = bufferSamples;
	
	//only use the last 1024 samples or less
	int32_t startIdx = ( endIdx - 1024 );
	startIdx = math<int32_t>::clamp( startIdx, 0, endIdx );
    
    float mx = -FLT_MAX,mn = FLT_MAX, val;
	for( uint32_t i = startIdx; i < endIdx; i++) {
        val = leftBuffer->mData[i];
        if (val > mx) mx = val;
        if (val < mn) mn = val;
	}
    
    float scale = 255.0/(mx - mn);
    for( uint32_t i = startIdx, c = 512; c < 1024; i++, c++ ) {
        signal[c] = (unsigned char) ((leftBuffer->mData[i]-mn)*scale);
	}
    
	// store it as a 512x2 texture
	soundTexture = std::make_shared<gl::Texture>( signal, GL_LUMINANCE, 512, 2 );
    
}

void BritneysSpaceshipApp::draw()
{
	gl::clear();
    
    if (soundTexture) soundTexture->enableAndBind();
    palletteTexture->enableAndBind();
    
	mShader->bind();
	mShader->uniform( "iChannel0", soundTexture ? 0: 1 );
    mShader->uniform( "iChannel1", 1 );
    mShader->uniform( "iResolution", Vec3f( getWindowWidth(), getWindowHeight(), 0.0f ) );
    mShader->uniform( "iGlobalTime", float( getElapsedSeconds() ) );
    
	gl::drawSolidRect( getWindowBounds() );
    
	if (soundTexture) soundTexture->unbind();
    
    palletteTexture->unbind();
}


CINDER_APP_BASIC( BritneysSpaceshipApp, RendererGl )