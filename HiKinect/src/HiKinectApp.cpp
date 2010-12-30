#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/params/Params.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "CinderOpenCV.h"
#include "SilhouetteDetector.h"

#include "GridMesh.h"
#include "Kinect.h"

using namespace ci;
using namespace ci::app;
using namespace std;

GLfloat no_mat[]			= { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_ambient[]		= { 0.6, 0.3, 0.4, 1.0 };
GLfloat mat_diffuse[]		= { 0.3, 0.5, 0.8, 1.0 };
GLfloat mat_specular[]		= { 1.0, 1.0, 1.0, 1.0 };
GLfloat mat_emission[]		= { 0.0, 0.1, 0.3, 0.0 };

GLfloat mat_shininess[]		= { 128.0 };
GLfloat no_shininess[]		= { 0.0 };

class HiKinectApp : public AppBasic {
  public:
	void prepareSettings( Settings* settings );
	void setup();
	void mouseDown( MouseEvent event );	
	void mouseUp( MouseEvent event );
	void mouseMove( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void keyDown( KeyEvent event );
	void update();
	void draw();
	void draw3D();
	void generateNormalMap();
	
	static const int	CAPTURE_WIDTH = 640, CAPTURE_HEIGHT = 480;
	
	Kinect				mKinect;
	bool				mKinectReady;
	bool				mKinectIR;
	gl::Texture			mColorTexture, mDepthTexture;
	Surface32f			mDepthSurface;
	Channel				mDepthChannel;
	
	SilhouetteDetector	*mSilhouetteDetector;
	vector< vector<cv::Point> > * mContours;
	gl::Texture			mContourMask;
	
	gl::GlslProg		mNormalShader;
	float				mNormalStrength;
	gl::Fbo				mFbo;
	
	params::InterfaceGl	mParams;
	
	Vec2f				mMousePos;
	bool				mIsMouseDown;
	bool				mWireframe;
	bool				mLighting;
	bool				mDoNormalMap;
	
	bool				DIFFUSE;
	bool				AMBIENT;
	bool				SPECULAR;
	bool				EMISSIVE;
	float				mDirectional;
	
	GridMesh			mGridMesh;
	CameraPersp			mCamera;
	MayaCamUI			mCamUI;
};

void HiKinectApp::prepareSettings( Settings *settings ) {
	settings->setWindowSize( 800, 600 );
	settings->setFrameRate( 60.0f );
}

void HiKinectApp::setup()
{
	console() << "There are " << Kinect::getNumDevices() << " Kinects connected." << std::endl;
	
	mKinect = Kinect( Kinect::Device() );
	mKinectReady = false;
	mKinectIR = false;
	
	mNormalShader = gl::GlslProg( loadResource( "normal_vert.glsl" ), loadResource( "normal_frag.glsl" ) );
	mNormalStrength = 20.0f;
	
	gl::Fbo::Format format;
	mFbo = gl::Fbo( CAPTURE_WIDTH, CAPTURE_HEIGHT, format );
	
	mSilhouetteDetector = new SilhouetteDetector(640,480);
	mContours = new vector<vector<cv::Point> >();
	
	mDoNormalMap = false;
	
	mParams = params::InterfaceGl( "Parameters", Vec2i( 300, 200 ) );
	mParams.addParam( "NormalStrength", &mNormalStrength, "min=1.0 max=1000.0 step=1.0 keyIncr=z keyDecr=Z" );
	mParams.addParam( "Depth Scale", &mGridMesh.mDepthScale, "min=1.0 max=2000.0" );
	mParams.addParam( "Depth Offset", &mGridMesh.mDepthOffset, "min=0.0 max=1000.0" );
	mParams.addParam( "Shader Displacement", &mGridMesh.mShaderDisplacement );
	mParams.addParam( "Depth Max", &mGridMesh.mDepthMax, "min=0.0 max=1.0 step=0.01" );
	mParams.addParam( "Generate Normal Map", &mDoNormalMap );
	
	mGridMesh.init(160, 120, 640, 480, false, true);
	mIsMouseDown = false;
	mWireframe = true;
	
	DIFFUSE		= true;
	AMBIENT		= false;
	SPECULAR	= false;
	EMISSIVE	= false;
	
	mCamera.setEyePoint( Vec3f(0.0f, 0.0f, 750.0f));
	mCamera.setCenterOfInterestPoint( Vec3f::zero() );
	mCamera.setPerspective( 60, getWindowAspectRatio(), 1, 2000 );
	mCamUI.setCurrentCam( mCamera );
	
	gl::enableDepthWrite();
	gl::enableDepthRead();
}

void HiKinectApp::mouseUp( MouseEvent event ) {
	mIsMouseDown = false;
}

void HiKinectApp::mouseDown( MouseEvent event ) {
	mIsMouseDown = true;
	mCamUI.mouseDown(event.getPos());
}

void HiKinectApp::mouseDrag( MouseEvent event ) {
	mouseMove( event );
	mCamUI.mouseDrag(event.getPos(), event.isLeftDown(), event.isControlDown(), event.isAltDown());
}

void HiKinectApp::mouseMove( MouseEvent event ) {
	mMousePos.x = event.getX() - getWindowWidth() * 0.5f;
	mMousePos.y = getWindowHeight() * 0.5f - event.getY();
}

void HiKinectApp::keyDown( KeyEvent event ) {
	
	if( event.getChar() == 'd' || event.getChar() == 'D' ){
		DIFFUSE = ! DIFFUSE;
	}
	else if( event.getChar() == 'a' || event.getChar() == 'A' ){
		AMBIENT = ! AMBIENT;
	}
	else if( event.getChar() == 's' || event.getChar() == 'S' ){
		SPECULAR = ! SPECULAR;
	}
	else if( event.getChar() == 'e' || event.getChar() == 'E' ){
		EMISSIVE = ! EMISSIVE;
	}
	else if( event.getChar() == 'f' || event.getChar() == 'F' ){
		setFullScreen( ! isFullScreen() );
	}
	else if( event.getChar() == ',' || event.getChar() == '<' ){
		mat_shininess[0] *= 0.5f;
		if( mat_shininess[0] < 8.0f )
			mat_shininess[0] = 8.0f;
	}
	else if( event.getChar() == '.' || event.getChar() == '>' ){
		mat_shininess[0] *= 2.0f;
		if( mat_shininess[0] > 128.0f )
			mat_shininess[0] = 128.0f;
	}
	else if (event.getChar() == 'w' || event.getChar() == 'W') {
		mWireframe = !mWireframe;
	}
	else if (event.getChar() == 'l' || event.getChar() == 'L') {
		mLighting = !mLighting;
	}
	
	else if (event.getCode() == 273) {
		console() << "doing something" << std::endl;
		mGridMesh.mTriIndex ++;
	}
	else if (event.getCode() == 274) {
		mGridMesh.mTriIndex --;
	}
}

void HiKinectApp::update()
{
	if( mKinect.checkNewDepthFrame() ) {
		mDepthTexture = mKinect.getDepthImage();
		mDepthSurface = Surface32f( mKinect.getDepthImage() );
		mKinectReady = true;
		if ( !mKinectIR ) {
			mKinectIR = true;
			mKinect.setVideoInfrared( true );
		}
		
		ci::Surface captureSurface = Surface8u( mKinect.getDepthImage() );
		ci::Surface outputSurface = captureSurface;
		mContours->clear();
		mSilhouetteDetector->processSurface(&captureSurface, mContours, &outputSurface);
	}
	
	if( mKinect.checkNewColorFrame() )
		mColorTexture = mKinect.getVideoImage();
	
	if( mIsMouseDown ) // using small number instead of 0.0 because lights go black after a few seconds when going to 0.0f
		mDirectional -= ( mDirectional - 0.00001f ) * 0.1f;  
	else 
		mDirectional -= ( mDirectional - 1.0f ) * 0.1f;
	
	if (mKinectReady)
		mGridMesh.updateKinect(mKinect);
	else
		mGridMesh.update();
}

void HiKinectApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::disableDepthWrite();
	gl::disableDepthRead();
	
	if (mDoNormalMap) {
		generateNormalMap();
		gl::setViewport( getWindowBounds() );
		gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );
		gl::draw( mFbo.getTexture(0), Rectf( 0, getWindowHeight(), getWindowWidth(), 0 ) );
	}
	
	
	gl::enableDepthWrite();
	gl::enableDepthRead();
	
	draw3D();
	
	params::InterfaceGl::draw();
}

void HiKinectApp::draw3D() {
	
	gl::setMatrices( mCamUI.getCamera() );
	if (mWireframe) gl::enableWireframe();
	else gl::disableWireframe();
	
	if (mLighting) {
		glEnable( GL_LIGHTING );
		glEnable( GL_LIGHT0 );
	}
//	GLfloat light_position[] = { mMousePos.x, mMousePos.y, -275.0f, 0.0f };
	GLfloat light_position[] = { 0, 0, 1.0f, 0.0f };
	glLightfv( GL_LIGHT0, GL_POSITION, light_position );
	if( DIFFUSE ){
		ci::ColorA color( CM_RGB, 1.0f, 1.0f, 1.0f, 1.0f );
		glMaterialfv( GL_FRONT, GL_DIFFUSE,	color );
	} else {
		glMaterialfv( GL_FRONT, GL_DIFFUSE,	no_mat );
	}
	
	if( AMBIENT )
		glMaterialfv( GL_FRONT, GL_AMBIENT,	mat_ambient );
	else
		glMaterialfv( GL_FRONT, GL_AMBIENT,	no_mat );
	
	if( SPECULAR ){
		glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular );
		glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess );
	} else {
		glMaterialfv( GL_FRONT, GL_SPECULAR, no_mat );
		glMaterialfv( GL_FRONT, GL_SHININESS, no_shininess );
	}
	
	if( EMISSIVE )
		glMaterialfv( GL_FRONT, GL_EMISSION, mat_emission );
	else
		glMaterialfv( GL_FRONT, GL_EMISSION, no_mat );
	
	if (mDepthTexture)
		mDepthTexture.bind(0);
	mFbo.bindTexture(1);
	if (mColorTexture)
		mColorTexture.bind(2);
	
	mGridMesh.draw( lmap(mMousePos.x, 0.0f, (float)getWindowWidth(), 0.0f, 1.0f) );
	
	if (mLighting) {
		glDisable( GL_LIGHTING );
		glDisable( GL_LIGHT0 );
	}
	
	gl::disableWireframe();
}

void HiKinectApp::generateNormalMap() {
	// bind the Fbo
	mFbo.bindFramebuffer();
	// match the viewport to the Fbo dimensions
	gl::setViewport( mFbo.getBounds() );
	// setup an ortho projection
	gl::setMatricesWindow( mFbo.getWidth(), mFbo.getHeight() );
	// clear the Fbo
	gl::clear( Color( 0, 0, 0 ) );
	
	// bind the shader, set its variables
	mNormalShader.bind();
	mNormalShader.uniform( "normalStrength", mNormalStrength);
	mNormalShader.uniform( "texelWidth", 1.0f / (float)CAPTURE_WIDTH );
	
	if ( mDepthTexture ) {
		gl::pushModelView();
//		gl::translate( Vec3f( 0, mDepthTexture.getHeight(), 0 ) );
//		gl::scale( Vec3f( 1, -1, 1 ) );
		gl::draw( mDepthTexture );
		gl::popModelView();
	}
	
	// unbind the shader
	mNormalShader.unbind();
	// unbind the Fbo
	mFbo.unbindFramebuffer();
}


CINDER_APP_BASIC( HiKinectApp, RendererGl )
