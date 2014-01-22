#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TestProjectApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void TestProjectApp::setup()
{
}

void TestProjectApp::mouseDown( MouseEvent event )
{
}

void TestProjectApp::update()
{
}

void TestProjectApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( TestProjectApp, RendererGl )
