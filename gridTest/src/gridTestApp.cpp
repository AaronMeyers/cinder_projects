#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "GridMesh.h"
#include "cinder/MayaCamUI.h"

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

class gridTestApp : public AppBasic {
  public:
	GridMesh			mGridMesh;
	CameraPersp			mCamera;
	CameraOrtho			mOrthoCamera;
	MayaCamUI			mCamUI;
	
	Vec2f				mMousePos;
	bool				mIsMouseDown;
	bool				mWireframe;
	bool				mLighting;
	
	bool DIFFUSE;
	bool AMBIENT;
	bool SPECULAR;
	bool EMISSIVE;
	float mDirectional;
	
	void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );	
	void mouseUp( MouseEvent event );
	void mouseMove( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void keyDown( KeyEvent event );
	void update();
	void draw();
};

void gridTestApp::prepareSettings( Settings *settings ) {
	settings->setWindowSize( 1024, 768 );
    settings->setFrameRate( 60.0f );
}

void gridTestApp::setup()
{
	mGridMesh.init(64, 48, 640, 480);
	mIsMouseDown = false;
	mWireframe = true;
	
	DIFFUSE		= true;
	AMBIENT		= false;
	SPECULAR	= false;
	EMISSIVE	= false;
	
	mDirectional = 1.0f;
	mLighting = true;
	
	mCamera.setEyePoint( Vec3f(0.0f, 0.0f, 750.0f));
	mCamera.setCenterOfInterestPoint( Vec3f::zero() );
	mCamera.setPerspective( 60, getWindowAspectRatio(), 1, 2000 );
	
	mCamUI.setCurrentCam( mCamera );
	
	gl::enableDepthWrite();
	gl::enableDepthRead();
}

void gridTestApp::mouseUp( MouseEvent event ) {
	mIsMouseDown = false;
}

void gridTestApp::mouseDown( MouseEvent event ) {
	mIsMouseDown = true;
	mCamUI.mouseDown(event.getPos());
}

void gridTestApp::mouseDrag( MouseEvent event ) {
	mouseMove( event );
	mCamUI.mouseDrag(event.getPos(), event.isLeftDown(), event.isControlDown(), event.isAltDown());
}

void gridTestApp::mouseMove( MouseEvent event ) {
	mMousePos.x = event.getX() - getWindowWidth() * 0.5f;
	mMousePos.y = getWindowHeight() * 0.5f - event.getY();
}

void gridTestApp::keyDown( KeyEvent event ) {
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

void gridTestApp::update()
{
	if( mIsMouseDown ) // using small number instead of 0.0 because lights go black after a few seconds when going to 0.0f
		mDirectional -= ( mDirectional - 0.00001f ) * 0.1f;  
	else 
		mDirectional -= ( mDirectional - 1.0f ) * 0.1f;
	
	gl::setMatrices( mCamUI.getCamera() );
	mGridMesh.update();
	
	if (mWireframe) gl::enableWireframe();
	else gl::disableWireframe();
}

void gridTestApp::draw()
{
	if (mLighting) {
		glEnable( GL_LIGHTING );
		glEnable( GL_LIGHT0 );
	}
	GLfloat light_position[] = { mMousePos.x, mMousePos.y, 275.0f, mDirectional };
//	GLfloat light_position[] = { 0, 0, 1.0f, 0.0f };
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
	
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
//	gl::enableWireframe();
	mGridMesh.draw( lmap(mMousePos.x, 0.0f, (float)getWindowWidth(), 0.0f, 1.0f) );
	
	if (mLighting) {
		glDisable( GL_LIGHTING );
		glDisable( GL_LIGHT0 );
	}
}


CINDER_APP_BASIC( gridTestApp, RendererGl )
