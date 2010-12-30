#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/params/Params.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ScreenShadersApp : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	
	static const int	CAPTURE_WIDTH = 640, CAPTURE_HEIGHT = 480;
	
	Capture				mCapture;
	gl::Fbo				mFbo;
	gl::Texture			mTexture;
	gl::GlslProg		mBlurShader;
	gl::Texture			mBlurKernel;
};

void ScreenShadersApp::setup()
{
	try {
		vector<Capture::DeviceRef> devices( Capture::getDevices() );
		mCapture = Capture( CAPTURE_WIDTH, CAPTURE_HEIGHT, devices[0] );
		mCapture.start();
	}
	catch( ... ) {
		console() << "Failed to initialize capture" << std::endl;
	}
	
	gl::Fbo::Format format;
	mFbo = gl::Fbo( CAPTURE_WIDTH, CAPTURE_HEIGHT, format );
	
	mBlurShader = gl::GlslProg( loadResource( "passThruVert.glsl" ), loadResource( "blurFrag.glsl" ) );
	mBlurKernel = gl::Texture( loadImage( loadResource( "kernel.png" ) ) );
}

void ScreenShadersApp::mouseDown( MouseEvent event )
{
}

void ScreenShadersApp::update()
{
	if ( mCapture && mCapture.checkNewFrame() ) {
		mTexture = gl::Texture( mCapture.getSurface() );
	}
}

void ScreenShadersApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	
	// bind the Fbo
	mFbo.bindFramebuffer();
	// match the viewport to the Fbo dimensions
	gl::setViewport( mFbo.getBounds() );
	// setup an ortho projection
	gl::setMatricesWindow( mFbo.getWidth(), mFbo.getHeight() );
	// clear the Fbo
	gl::clear( Color( 0, 0, 0 ) );
	
	if ( mTexture ) {
		gl::draw( mTexture );
	}
	
	// unbind the Fbo
	mFbo.unbindFramebuffer();
	
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
	
	float kernelRes = 21.0f;
	
	mBlurShader.bind();
	mBlurShader.uniform( "kernelRes", kernelRes );
	mBlurShader.uniform( "invKernelRes", 1.0f / kernelRes );
	mBlurShader.uniform( "fboTex", 0 );
	mBlurShader.uniform( "kernelTex", 1 );
	mBlurShader.uniform( "orientationVector", Vec2f( 1.0f, 1.0f ) );
	mBlurShader.uniform( "blurAmt", 1.0f );
	mBlurShader.uniform( "colMulti", 1.0f );
	
	mBlurKernel.bind( 1 );
	
	gl::draw( mFbo.getTexture(0), Rectf( 0, getWindowHeight(), getWindowWidth(), 0 ) );
	mBlurShader.unbind();
}


CINDER_APP_BASIC( ScreenShadersApp, RendererGl )
