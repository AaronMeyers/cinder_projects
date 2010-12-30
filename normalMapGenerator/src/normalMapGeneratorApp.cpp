#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class normalMapGeneratorApp : public AppBasic {
public:
	void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );	
	void mouseMove( MouseEvent event );
	void update();
	void draw();
	
	static const int	CAPTURE_WIDTH = 640, CAPTURE_HEIGHT = 480;
	
	Capture				mCapture;
	gl::Texture			mTexture;
	gl::GlslProg		mShader;
	float				mNormalStrength;
	gl::Fbo				mFbo;
	Vec2f				mMousePos;
	
	params::InterfaceGl	mParams;
};

void normalMapGeneratorApp::prepareSettings( Settings *settings ) {
	settings->setWindowSize( 1024, 768 );
    settings->setFrameRate( 60.0f );	
}

void normalMapGeneratorApp::setup()
{
	try {
		vector<Capture::DeviceRef> devices( Capture::getDevices() );
		mCapture = Capture( CAPTURE_WIDTH, CAPTURE_HEIGHT, devices[0] );
		mCapture.start();
	}
	catch( ... ) {
		console() << "Failed to initialize capture" << std::endl;
	}
	
	mShader = gl::GlslProg( loadResource( "normal_vert.glsl" ), loadResource( "normal_frag.glsl" ) );
	mNormalStrength = 2.0f;
	
	gl::Fbo::Format format;
	mFbo = gl::Fbo( CAPTURE_WIDTH, CAPTURE_HEIGHT, format );
	
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 400 ) );
	mParams.addParam( "Normal Strength", &mNormalStrength, "min=0.0 max=10.0" );
}

void normalMapGeneratorApp::mouseMove( MouseEvent event ) {
	mMousePos.set( event.getPos() );
}

void normalMapGeneratorApp::mouseDown( MouseEvent event ) {
}

void normalMapGeneratorApp::update()
{
	if ( mCapture && mCapture.checkNewFrame() ) {
		mTexture = gl::Texture( mCapture.getSurface() );
	}
}

void normalMapGeneratorApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	
	// bind the Fbo
	mFbo.bindFramebuffer();
	// match the viewport to the Fbo dimensions
	gl::setViewport( mFbo.getBounds() );
	// setup an ortho projection
	gl::setMatricesWindow( mFbo.getWidth(), mFbo.getHeight() );
	// clear the Fbo
	gl::clear( Color( 0, 0, 0 ) );
	
	// bind the shader, set its variables
	mShader.bind();
	mShader.uniform( "normalStrength", mNormalStrength);
	mShader.uniform( "texelWidth", 1.0f / (float)mCapture.getWidth() );
	
	// draw
	if ( mTexture ) {
		gl::pushModelView();
		gl::translate( Vec3f( 0, mTexture.getHeight(), 0 ) );
		gl::scale( Vec3f( 1, -1, 1 ) );
		gl::draw( mTexture );
		gl::popModelView();
	}
	
	// unbind the shader
	mShader.unbind();
	// unbind the Fbo
	mFbo.unbindFramebuffer();
	
	gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
//	gl::draw( mFbo.getTexture(0), Rectf( mMousePos, mMousePos + mFbo.getSize() ) );
	gl::draw( mFbo.getTexture(0), Rectf( 0, 0, getWindowWidth(), getWindowHeight() ) );
	
	params::InterfaceGl::draw();
}


CINDER_APP_BASIC( normalMapGeneratorApp, RendererGl )
