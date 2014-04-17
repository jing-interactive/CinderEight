#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AllInOneIOSApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void AllInOneIOSApp::setup()
{
}

void AllInOneIOSApp::mouseDown( MouseEvent event )
{
}

void AllInOneIOSApp::update()
{
}

void AllInOneIOSApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( AllInOneIOSApp, RendererGl )
