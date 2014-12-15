#include "cinder/Camera.h"
#include "cinder/GeomIo.h"
#include "cinder/ImageIo.h"
#include "cinder/MayaCamUI.h"
#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/params/Params.h"
#include "cinder/Log.h"
#include "Resources.h"
#include "cinder/audio/MonitorNode.h"
#include "cinder/audio/Device.h"

#define INPUT_DEVICE "Scarlett 2i2 USB"

using namespace ci;
using namespace ci::app;
using namespace std;

class DynamicGeometryApp : public AppNative {
  public:
	enum Primitive { CAPSULE, PLANE };
	enum Quality { LOW, DEFAULT, HIGH };
	enum ViewMode { SHADED, WIREFRAME };
	enum TexturingMode { NONE, PROCEDURAL, SAMPLER };

	void prepareSettings( Settings *settings ) override;
	void setup() override;
	void resize() override;
	void update() override;
	void draw() override;

	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void keyDown( KeyEvent event ) override;

  private:
	void createGrid();
	void createPhongShader();
	void createWireframeShader();
	void createGeometry();
	void loadGeomSource( const geom::Source &source );
	void createParams();

	Primitive			mPrimitiveSelected;
	Primitive			mPrimitiveCurrent;
	Quality				mQualitySelected;
	Quality				mQualityCurrent;
	ViewMode			mViewMode;

	int					mSubdivision;
	int					mTexturingMode;

	bool				mShowColors;
	bool				mShowNormals;
	bool				mShowGrid;
	bool				mEnableFaceFulling;
    
    float               mCapsuleLength;

	CameraPersp			mCamera;
	MayaCamUI			mMayaCam;
	bool				mRecenterCamera;
	vec3				mCameraCOI;
	double				mLastMouseDownTime;

	gl::VertBatchRef	mGrid;

	gl::BatchRef		mPrimitive;
	gl::BatchRef		mPrimitiveWireframe;
	gl::BatchRef		mPrimitiveNormalLines;

	gl::GlslProgRef		mPhongShader;
	gl::GlslProgRef		mWireframeShader;
    gl::GlslProgRef     mShader;

	gl::TextureRef		mTexture;
    AxisAlignedBox3f    mBbox;
    
    float               mFrameRate;
	
#if ! defined( CINDER_GL_ES )
	params::InterfaceGlRef	mParams;
#endif
    
    
    audio::InputDeviceNodeRef		mInputDeviceNode;
    audio::MonitorSpectralNodeRef	mMonitorSpectralNode;
    vector<float>					mMagSpectrum;
    
    // width and height of our mesh
    static const int kWidth = 256;
    static const int kHeight = 256;
    
    // number of frequency bands of our spectrum
    static const int kBands = 1024;
    static const int kHistory = 128;
    
    Channel32f			mChannelLeft;
    Channel32f			mChannelRight;
    gl::TextureRef			mTextureLeft;
    gl::TextureRef		mTextureRight;
    gl::Texture::Format	mTextureFormat;
    uint32_t			mOffset;
};

void DynamicGeometryApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize(1024, 768);
	settings->enableHighDensityDisplay();
	settings->enableMultiTouch( false );
}

void DynamicGeometryApp::setup()
{
	// Initialize variables.
    mOffset = 0;
    mFrameRate = 0.f;
	mPrimitiveSelected = mPrimitiveCurrent = PLANE;
	mQualitySelected = mQualityCurrent = HIGH;
	mTexturingMode = PROCEDURAL;
	mViewMode = SHADED;
	mLastMouseDownTime = 0;
	mShowColors = false;
	mShowNormals = false;
	mShowGrid = true;
	mEnableFaceFulling = false;

	mSubdivision = 1;
    
    mCapsuleLength = 0.01f;
	
	// Load the textures.
	gl::Texture::Format fmt;
	fmt.setAutoInternalFormat();
	fmt.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	mTexture = gl::Texture::create( loadImage( loadResource( RES_LANDSCAPE_IMAGE)  ), fmt );

	// Setup the camera.
	mCamera.setEyePoint( normalize( vec3( 3, 3, 6 ) ) * 5.0f );
	mCamera.setCenterOfInterestPoint( mCameraCOI );

	// Load and compile the shaders.
	createPhongShader();
	createWireframeShader();

	// Create the meshes.
	createGrid();
	createGeometry();

	// Enable the depth buffer.
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// Create a parameter window, so we can toggle stuff.
	createParams();

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
    
    cout<< "Using " << mInputDeviceNode->getDevice() -> getName() << endl;
    
    // By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
    // an increase in resolution of the resulting spectrum data.
    auto monitorFormat = audio::MonitorSpectralNode::Format().fftSize( kBands ).windowSize( kBands / 2 );
    mMonitorSpectralNode = ctx->makeNode( new audio::MonitorSpectralNode( monitorFormat ) );
    
    mInputDeviceNode >> mMonitorSpectralNode;
    
    // InputDeviceNode (and all InputNode subclasses) need to be enabled()'s to process audio. So does the Context:
    mInputDeviceNode->enable();
    ctx->enable();
    
    mChannelLeft = Channel32f(kBands, kHistory);
    mChannelRight = Channel32f(kBands, kHistory);
    memset(	mChannelLeft.getData(), 0, mChannelLeft.getRowBytes() * kHistory );
    memset(	mChannelRight.getData(), 0, mChannelRight.getRowBytes() * kHistory );

    mTextureFormat.setWrapS( GL_CLAMP_TO_BORDER );
    mTextureFormat.setWrapT( GL_REPEAT );
    mTextureFormat.setMinFilter( GL_LINEAR );
    mTextureFormat.setMagFilter( GL_LINEAR );

}

void DynamicGeometryApp::update()
{
    
    mFrameRate = getAverageFps();
	// If another primitive or quality was selected, reset the subdivision and recreate the primitive.
	if( mPrimitiveCurrent != mPrimitiveSelected || mQualitySelected != mQualityCurrent ) {
		mSubdivision = 1;
		mPrimitiveCurrent = mPrimitiveSelected;
		mQualityCurrent = mQualitySelected;
		createGeometry();
	}

	// After creating a new primitive, gradually move the camera to get a good view.
	if( mRecenterCamera ) {
		float distance = glm::distance( mCamera.getEyePoint(), mCameraCOI );
		mCamera.setEyePoint( mCameraCOI - lerp( distance, 5.0f, 0.1f ) * mCamera.getViewDirection() );
		mCamera.setCenterOfInterestPoint( lerp( mCamera.getCenterOfInterestPoint(), mCameraCOI, 0.25f) );
	}
    
    
    mMagSpectrum = mMonitorSpectralNode->getMagSpectrum();
    
    // get spectrum for left and right channels and copy it into our channels
    float* pDataLeft = mChannelLeft.getData() + kBands * mOffset;
    float* pDataRight = mChannelRight.getData() + kBands * mOffset;
    
    std::reverse_copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataLeft);
    std::copy(mMagSpectrum.begin(), mMagSpectrum.end(), pDataRight);
    
    // increment texture offset
    mOffset = (mOffset+1) % kHistory;
    
    mTextureLeft = gl::Texture::create(mChannelLeft, mTextureFormat);
    mTextureRight = gl::Texture::create(mChannelRight, mTextureFormat);

}

void DynamicGeometryApp::draw()
{
	// Prepare for drawing.
	gl::clear( Color::black() );
	gl::setMatrices( mCamera );
	
	// Draw the grid.
	if( mShowGrid && mGrid ) {
		gl::ScopedGlslProg scopedGlslProg( gl::context()->getStockShader( gl::ShaderDef().color() ) );
		// draw the coordinate frame with length 2.
		gl::drawCoordinateFrame( 2 );
		mGrid->draw();
        
        gl::drawVector(mBbox.getCenter(), mCamera.getCenterOfInterestPoint());
	}

	if( mPrimitive ) {
        gl::setDefaultShaderVars();
		gl::ScopedTextureBind scopedTextureBind( mTexture );
        //gl::ScopedGlslProg sh(mShader);
		mPhongShader->uniform( "uTexturingMode", mTexturingMode );

		// Rotate it slowly around the y-axis.
		gl::pushModelView();
		gl::rotate( float( getElapsedSeconds() / 10 ), 0.0f, 1.0f, 0.0f );

		// Draw the normals.
		if( mShowNormals && mPrimitiveNormalLines ) {
			gl::ScopedColor colorScope( Color( 1, 1, 0 ) );
			mPrimitiveNormalLines->draw();
		}

		// Draw the primitive.
		gl::ScopedColor colorScope( Color( 0.7f, 0.5f, 0.3f ) );

		// (If transparent, render the back side first).
		if( mViewMode == WIREFRAME ) {
			gl::enableAlphaBlending();

			gl::enable( GL_CULL_FACE );
			gl::cullFace( GL_FRONT );

			mWireframeShader->uniform( "uBrightness", 0.5f );
			mPrimitiveWireframe->draw();
		}

		// (Now render the front side.)
		if( mViewMode == WIREFRAME ) {
			gl::cullFace( GL_BACK );

			mWireframeShader->uniform( "uBrightness", 1.0f );
			mPrimitiveWireframe->draw();
			
			gl::disable( GL_CULL_FACE );

			gl::disableAlphaBlending();
		}
		else {
            float off = (mOffset / float(kHistory) - 0.5) * 2.0f;
            mShader->uniform("uTexOffset", off);
            mShader->uniform("time", (float)getElapsedSeconds() * 0.001f);
            mShader->uniform("resolution", 0.25f*(float)kWidth);
            mShader->uniform("uTex0",0);
            mShader->uniform("uLeftTex", 1);
            mShader->uniform("uRightTex", 2);

            gl::ScopedTextureBind texLeft( mTextureLeft, 1 );
            gl::ScopedTextureBind texRight( mTextureRight, 2 );

            gl::ScopedAdditiveBlend blend;
			mPrimitive->draw();
        }
		
		// Done.
		gl::popModelView();
	}

	// Render the parameter window.
#if ! defined( CINDER_GL_ES )
	if( mParams )
		mParams->draw();
#endif
}

void DynamicGeometryApp::mouseDown( MouseEvent event )
{
	mRecenterCamera = false;

	mMayaCam.setCurrentCam( mCamera );
	mMayaCam.mouseDown( event.getPos() );

	if( getElapsedSeconds() - mLastMouseDownTime < 0.2f ) {
		mPrimitiveSelected = static_cast<Primitive>( static_cast<int>(mPrimitiveSelected) + 1 );
		createGeometry();
	}

	mLastMouseDownTime = getElapsedSeconds();
}

void DynamicGeometryApp::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
	mCamera = mMayaCam.getCamera();
}

void DynamicGeometryApp::resize()
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
	
	if(mWireframeShader)
		mWireframeShader->uniform( "uViewportSize", vec2( getWindowSize() ) );
}

void DynamicGeometryApp::keyDown( KeyEvent event )
{
	switch( event.getCode() ) {
		case KeyEvent::KEY_SPACE:
			mPrimitiveSelected = static_cast<Primitive>( static_cast<int>(mPrimitiveSelected) + 1 );
			createGeometry();
			break;
		case KeyEvent::KEY_c:
			mShowColors = ! mShowColors;
			createGeometry();
			break;
		case KeyEvent::KEY_n:
			mShowNormals = ! mShowNormals;
			break;
		case KeyEvent::KEY_g:
			mShowGrid = ! mShowGrid;
			break;
		case KeyEvent::KEY_q:
			mQualitySelected = Quality( (int)( mQualitySelected + 1 ) % 3 );
			break;
		case KeyEvent::KEY_w:
			if(mViewMode == WIREFRAME)
				mViewMode = SHADED;
			else
				mViewMode = WIREFRAME;
			break;
		case KeyEvent::KEY_f:
			setFullScreen(!isFullScreen());
			break;
		case KeyEvent::KEY_RETURN:
			CI_LOG_V( "reload" );
			createPhongShader();
			createGeometry();
			break;
	}
}

void DynamicGeometryApp::createParams()
{
#if ! defined( CINDER_GL_ES )
	vector<string> primitives = { "Capsule","Plane" };
	vector<string> qualities = { "Low", "Default", "High" };
	vector<string> viewModes = { "Shaded", "Wireframe" };
	vector<string> texturingModes = { "None", "Procedural", "Sampler" };

	mParams = params::InterfaceGl::create( getWindow(), "Dynamic Geometry", toPixels( ivec2( 300, 200 ) ) );
	mParams->setOptions( "", "valueswidth=160 refresh=0.1" );
    
    mParams->addParam("FPS", &mFrameRate, true);

	mParams->addParam( "Primitive", primitives, (int*) &mPrimitiveSelected );
	mParams->addParam( "Quality", qualities, (int*) &mQualitySelected );
	mParams->addParam( "Viewing Mode", viewModes, (int*) &mViewMode );
	mParams->addParam( "Texturing Mode", texturingModes, (int*) &mTexturingMode );

	mParams->addSeparator();

	mParams->addParam( "Subdivision", &mSubdivision ).min( 1 ).max( 50 ).updateFn( [this] { createGeometry(); } );

	mParams->addSeparator();

	mParams->addParam( "Show Grid", &mShowGrid );
	mParams->addParam( "Show Normals", &mShowNormals );
	mParams->addParam( "Show Colors", &mShowColors ).updateFn( [this] { createGeometry(); } );
	mParams->addParam( "Face Culling", &mEnableFaceFulling ).updateFn( [this] { gl::enableFaceCulling( mEnableFaceFulling ); } );
    
    mParams->addSeparator();
    
    mParams->addParam( "Capsule length", &mCapsuleLength).min( 0.0f ).max( 1.0f ).step( 0.01f ).updateFn( [this] { createGeometry(); } );
#endif
}

void DynamicGeometryApp::createGrid()
{
	mGrid = gl::VertBatch::create( GL_LINES );
	mGrid->begin( GL_LINES );
	for( int i = -10; i <= 10; ++i ) {
		if( i == 0 )
			continue;

		mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
		mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
		mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
		mGrid->color( Color( 0.25f, 0.25f, 0.25f ) );
		
		mGrid->vertex( float(i), 0.0f, -10.0f );
		mGrid->vertex( float(i), 0.0f, +10.0f );
		mGrid->vertex( -10.0f, 0.0f, float(i) );
		mGrid->vertex( +10.0f, 0.0f, float(i) );
	}
	mGrid->end();
}

void DynamicGeometryApp::createGeometry()
{
	geom::SourceRef primitive;

	switch( mPrimitiveCurrent ) {
		default:
			mPrimitiveSelected = PLANE;
		case CAPSULE:
			switch( mQualityCurrent ) {
				case DEFAULT:	loadGeomSource( geom::Capsule( geom::Capsule() ).length(0.5) ); break;
				case LOW:		loadGeomSource( geom::Capsule().subdivisionsAxis( 6 ).subdivisionsHeight( 1 ) ); break;
				case HIGH:		loadGeomSource( geom::Capsule().subdivisionsAxis( 60 ).subdivisionsHeight( 20 ).length(mCapsuleLength) ); break;
			}
			break;
		case PLANE:
			ivec2 numSegments;
			switch( mQualityCurrent ) {
				case DEFAULT:	numSegments = ivec2( 10, 10 ); break;
				case LOW:		numSegments = ivec2( 2, 2 ); break;
				case HIGH:		numSegments = ivec2( 100, 100 ); break;
			}

			auto plane = geom::Plane().subdivisions( numSegments );

//			plane.normal( vec3( 0, 0, 1 ) ); // change the normal angle of the plane
//			plane.axes( vec3( 0.70710678118, -0.70710678118, 0 ), vec3( 0.70710678118, 0.70710678118, 0 ) ); // dictate plane u/v axes directly
//			plane.subdivisions( ivec2( 3, 10 ) ).size( vec2( 0.5f, 2.0f ) ).origin( vec3( 0, 1.0f, 0 ) ).normal( vec3( 0, 0, 1 ) ); // change the size and origin so that it is tall and thin, above the y axis.

			loadGeomSource( geom::Plane( plane ) );
			break;
	}
}

void DynamicGeometryApp::loadGeomSource( const geom::Source &source )
{
	// The purpose of the TriMesh is to capture a bounding box; without that need we could just instantiate the Batch directly using primitive
	TriMesh::Format fmt = TriMesh::Format().positions().normals().texCoords();
	if( mShowColors && source.getAvailableAttribs().count( geom::COLOR ) > 0 )
		fmt.colors();

	TriMesh mesh( source, fmt );
	mBbox = mesh.calcBoundingBox();
	mCameraCOI = mesh.calcBoundingBox().getCenter();
	mRecenterCamera = true;

	if( mSubdivision > 1 )
		mesh.subdivide( mSubdivision );

	if( mPhongShader )
		mPrimitive = gl::Batch::create( mesh, mShader );

	if( mWireframeShader )
		mPrimitiveWireframe = gl::Batch::create( mesh, mWireframeShader );

	vec3 size = mBbox.getMax() - mBbox.getMin();
	float scale = std::max( std::max( size.x, size.y ), size.z ) / 25.0f;
	mPrimitiveNormalLines = gl::Batch::create( geom::VertexNormalLines( mesh, scale ), gl::getStockShader( gl::ShaderDef().color() ) );

	getWindow()->setTitle( "Geometry - " + to_string( mesh.getNumVertices() ) + " vertices" );
}

void DynamicGeometryApp::createPhongShader()
{
	try {
#if defined( CINDER_GL_ES )
		mPhongShader = gl::GlslProg::create( loadAsset( "phong_es2.vert" ), loadAsset( "phong_es2.frag" ) );
#else
		mPhongShader = gl::GlslProg::create( loadAsset( "phong.vert" ), loadAsset( "phong.frag" ) );
        mShader = gl::GlslProg::create( loadResource( GLSL_VERT2 ), loadResource( GLSL_FRAG1 ) );
#endif
	}
	catch( Exception &exc ) {
		CI_LOG_E( "error loading phong shader: " << exc.what() );
	}
}

void DynamicGeometryApp::createWireframeShader()
{
#if ! defined( CINDER_GL_ES )
	try {
		auto format = gl::GlslProg::Format()
			.vertex( loadAsset( "wireframe.vert" ) )
			.geometry( loadAsset( "wireframe.geom" ) )
			.fragment( loadAsset( "wireframe.frag" ) );

		mWireframeShader = gl::GlslProg::create( format );
	}
	catch( Exception &exc ) {
		CI_LOG_E( "error loading wireframe shader: " << exc.what() );
	}
#endif // ! defined( CINDER_GL_ES )
}

CINDER_APP_NATIVE( DynamicGeometryApp, RendererGl )
