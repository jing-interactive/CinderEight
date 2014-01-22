
#include "cinder/Camera.h"
#include "cinder/ImageIo.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BloomingNeonApp : public AppBasic {
 public:
	 void prepareSettings( Settings *settings );

	 void setup();
	 void update();
	 void draw();

	 void render();

	 void keyDown( KeyEvent event );

	 void drawStrokedRect( const Rectf &rect );
protected:
	gl::Fbo			mFboScene;
	gl::Fbo			mFboBlur1;
	gl::Fbo			mFboBlur2;

	gl::GlslProg	mShaderBlur;
	gl::GlslProg	mShaderPhong;
	
	gl::Texture		mTexture;
	TriMesh			mMesh;
	Matrix44f		mTransform;
	CameraPersp		mCamera;
};

void BloomingNeonApp::prepareSettings( Settings *settings )
{
	settings->setResizable(false);
	settings->setWindowSize(780 + 256, 256);
	settings->setTitle("Blurred Neon Effect Using Fbo's and Shaders");
}

void BloomingNeonApp::setup()
{
	// setup our scene Fbo
	mFboScene = gl::Fbo(512, 512);

	// setup our blur Fbo's, smaller ones will generate a bigger blur
	mFboBlur1 = gl::Fbo(512/8, 512/8);
	mFboBlur2 = gl::Fbo(512/8, 512/8);

	// load and compile the shaders
	try { 
		mShaderBlur = gl::GlslProg( loadAsset("blur_vert.glsl"), loadAsset("blur_frag.glsl")); 
		mShaderPhong = gl::GlslProg( loadAsset("phong_vert.glsl"), loadAsset("phong_frag.glsl")); 
	} 
	catch( const std::exception &e ) { 
		console() << e.what() << endl; 
		quit();
	}

	// setup the stuff to render our ducky
	mTransform.setToIdentity();

	gl::Texture::Format format;
	format.enableMipmapping(true);

	mTexture = gl::Texture( loadImage( loadAsset("ducky.png") ), format );

	mMesh.read( loadAsset("ducky.msh") );

	mCamera.setEyePoint( Vec3f(2.5f, 5.0f, 5.0f) );
	mCamera.setCenterOfInterestPoint( Vec3f(0.0f, 2.0f, 0.0f) );
	mCamera.setPerspective( 60.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
}

void BloomingNeonApp::update()
{
	mTransform.setToIdentity();
	mTransform.rotate( Vec3f::xAxis(), sinf( (float) getElapsedSeconds() * 3.0f ) * 0.08f );
	mTransform.rotate( Vec3f::yAxis(), (float) getElapsedSeconds() * 0.1f );
	mTransform.rotate( Vec3f::zAxis(), sinf( (float) getElapsedSeconds() * 4.3f ) * 0.09f );
}

void BloomingNeonApp::draw()
{
	// clear our window
	gl::clear( Color::black() );

	// store our viewport, so we can restore it later
	Area viewport = gl::getViewport();

	// render a simple scene into mFboScene
	gl::setViewport( mFboScene.getBounds() );
	mFboScene.bindFramebuffer();
		gl::pushMatrices();
			gl::setMatricesWindow(512, 512, false);
			gl::clear( Color::black() );
			render();
		gl::popMatrices();
	mFboScene.unbindFramebuffer();

	// bind the blur shader
	mShaderBlur.bind();
	mShaderBlur.uniform("tex0", 0); // use texture unit 0
 
	// tell the shader to blur horizontally and the size of 1 pixel
	mShaderBlur.uniform("sampleOffset", Vec2f(1.0f/mFboBlur1.getWidth(), 0.0f));

	// copy a horizontally blurred version of our scene into the first blur Fbo
	gl::setViewport( mFboBlur1.getBounds() );
	mFboBlur1.bindFramebuffer();
		mFboScene.bindTexture(0);
		gl::pushMatrices();
			gl::setMatricesWindow(512, 512, false);
			gl::clear( Color::black() );
			gl::drawSolidRect( mFboBlur1.getBounds() );
		gl::popMatrices();
		mFboScene.unbindTexture();
	mFboBlur1.unbindFramebuffer();	
 
	// tell the shader to blur vertically and the size of 1 pixel
	mShaderBlur.uniform("sampleOffset", Vec2f(0.0f, 1.0f/mFboBlur2.getHeight()));

	// copy a vertically blurred version of our blurred scene into the second blur Fbo
	gl::setViewport( mFboBlur2.getBounds() );
	mFboBlur2.bindFramebuffer();
		mFboBlur1.bindTexture(0);
		gl::pushMatrices();
			gl::setMatricesWindow(512, 512, false);
			gl::clear( Color::black() );
			gl::drawSolidRect( mFboBlur2.getBounds() );
		gl::popMatrices();
		mFboBlur1.unbindTexture();
	mFboBlur2.unbindFramebuffer();

	// unbind the shader
	mShaderBlur.unbind();

	// restore the viewport
	gl::setViewport( viewport );

	// because the Fbo's have their origin in the LOWER-left corner,
	// flip the Y-axis before drawing
	gl::pushModelView();
	gl::translate( Vec2f(0, 256) );
	gl::scale( Vec3f(1, -1, 1) );

	// draw the 3 Fbo's 
	gl::color( Color::white() );
	gl::draw( mFboScene.getTexture(), Rectf(0, 0, 256, 256) );
	drawStrokedRect( Rectf(0, 0, 256, 256) );

	gl::draw( mFboBlur1.getTexture(), Rectf(260, 0, 260 + 256, 256) );
	drawStrokedRect( Rectf(260, 0, 260 + 256, 256) );

	gl::draw( mFboBlur2.getTexture(), Rectf(520, 0, 520 + 256, 256) );
	drawStrokedRect( Rectf(520, 0, 520 + 256, 256) );

	// draw our scene with the blurred version added as a blend
	gl::color( Color::white() );
	gl::draw( mFboScene.getTexture(), Rectf(780, 0, 780 + 256, 256) );

	gl::enableAdditiveBlending();
	gl::draw( mFboBlur2.getTexture(), Rectf(780, 0, 780 + 256, 256) );
	gl::disableAlphaBlending();
	drawStrokedRect( Rectf(780, 0, 780 + 256, 256) );
	
	// restore the modelview matrix
	gl::popModelView();

	// draw info
	gl::enableAlphaBlending();
	gl::drawStringCentered("Basic Scene", Vec2f(128, 236));
	gl::drawStringCentered("First Blur Pass (Horizontal)", Vec2f(260 + 128, 236));
	gl::drawStringCentered("Second Blur Pass (Vertical)", Vec2f(520 + 128, 236));
	gl::drawStringCentered("Final Scene", Vec2f(780 + 128, 236));
	gl::disableAlphaBlending();
}

void BloomingNeonApp::render()
{
	// get the current viewport
	Area viewport = gl::getViewport();

	// adjust the aspect ratio of the camera
	mCamera.setAspectRatio( viewport.getWidth() / (float) viewport.getHeight() );
	
	// render our scene (see the Picking3D sample for more info)
	gl::pushMatrices();

		gl::setMatrices( mCamera );
		gl::enableDepthRead();
		gl::enableDepthWrite();
		
		mTexture.enableAndBind();
			mShaderPhong.bind();
			mShaderPhong.uniform("tex0", 0);
				gl::pushModelView();
				gl::multModelView( mTransform );
					gl::color( Color::white() );
					gl::draw( mMesh );
				gl::popModelView();		
			mShaderPhong.unbind();
		mTexture.unbind();

		gl::disableDepthWrite();
		gl::disableDepthRead();

	gl::popMatrices();
}

void BloomingNeonApp::keyDown( KeyEvent event )
{
	switch( event.getCode() )
	{
	case KeyEvent::KEY_ESCAPE:
		quit();
		break;
	}
}

void BloomingNeonApp::drawStrokedRect( const Rectf &rect )
{
	// we don't want any texture on our lines
	glDisable(GL_TEXTURE_2D);

	gl::drawLine(rect.getUpperLeft(), rect.getUpperRight());
	gl::drawLine(rect.getUpperRight(), rect.getLowerRight());
	gl::drawLine(rect.getLowerRight(), rect.getLowerLeft());
	gl::drawLine(rect.getLowerLeft(), rect.getUpperLeft());
}

CINDER_APP_BASIC( BloomingNeonApp, RendererGl )