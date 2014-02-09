#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "cinder/audio/Input.h"

#include "cinder/app/AppBasic.h"
#include "cinder/audio/FftProcessor.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class AudioShaderApp : public AppNative {
public:
	void setup();
	void update();
	void draw();
    
private:
	gl::GlslProg	mShader;
	gl::Texture		mTexture;
    audio::Input mInput;
    std::shared_ptr<float> mFftDataRef;
	audio::PcmBuffer32fRef mPcmBuffer;
};

void AudioShaderApp::setup()
{
	// load and compile shader
	try {
		mShader = gl::GlslProg( loadResource("audio_surf_vert.glsl"), loadResource("audio_surf_frag.glsl") );
	}
	catch( const std::exception& e )
	{
		console() << e.what() << std::endl;
	}
    
    
    //iterate input devices and print their names to the console
	const std::vector<audio::InputDeviceRef>& devices = audio::Input::getDevices();
	for( std::vector<audio::InputDeviceRef>::const_iterator iter = devices.begin(); iter != devices.end(); ++iter ) {
		console() << (*iter)->getName() << std::endl;
	}
    
	//initialize the audio Input, using the default input device
	mInput = audio::Input();
	
	//tell the input to start capturing audio
	mInput.start();
}

void AudioShaderApp::update()
{
    
    mPcmBuffer = mInput.getPcmBuffer();
	if( ! mPcmBuffer ) {
		return;
	}

	uint16_t bandCount = 512;
	mFftDataRef = audio::calculateFft( mPcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ), bandCount );

    float * fftBuffer = mFftDataRef.get();
    if (!fftBuffer) return;
    	float ht = 1000.0f;
	// create a random FFT signal for test purposes
	unsigned char signal[1024];
	for(int i=0;i<512;++i)
		signal[i] = (unsigned char) (fftBuffer[i] / bandCount * ht);
	
	// add an audio signal for test purposes
	for(int i=0;i<512;++i)
		signal[512+i] = (unsigned char) (fftBuffer[i] / bandCount * ht);
    
	// store it as a 512x2 texture
	mTexture = gl::Texture( signal, GL_LUMINANCE, 512, 2 );
}

void AudioShaderApp::draw()
{
    if (!mTexture) return;
	gl::clear();
    
	// bind texture to slot 0
	mTexture.enableAndBind();
    
	// bind shader and set uniforms
	mShader.bind();
	mShader.uniform( "iResolution", Vec3f( getWindowWidth(), getWindowHeight(), 0.0f ) );
	mShader.uniform( "iGlobalTime", float( getElapsedSeconds() ) );
	mShader.uniform( "iChannel0", 0 );
    
	// run full screen pass
	gl::drawSolidRect( getWindowBounds() );
    
	// unbind shader and texture
	mShader.unbind();
	mTexture.unbind();
}

CINDER_APP_BASIC( AudioShaderApp, RendererGl );
