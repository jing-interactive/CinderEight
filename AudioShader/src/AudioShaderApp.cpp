#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"

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
}

void AudioShaderApp::update()
{
	// create a random FFT signal for test purposes
	unsigned char signal[1024];
	for(int i=0;i<512;++i)
		signal[i] = (unsigned char) (Rand::randUint() & 0xFF);
	
	// add an audio signal for test purposes
	for(int i=0;i<512;++i)
		signal[512+i] = (unsigned char) (Rand::randUint() & 0xFF);
    
	// store it as a 512x2 texture
	mTexture = gl::Texture( signal, GL_LUMINANCE, 512, 2 );
}

void AudioShaderApp::draw()
{
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
