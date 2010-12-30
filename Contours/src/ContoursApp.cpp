#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "CinderOpenCV.h"
#include "cinder/params/Params.h"

#include "Kinect.h"
#include "SilhouetteDetector.h"
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;

class ContoursApp : public AppBasic {
  public:
	void prepareSettings( Settings *settings);
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	
	Kinect				mKinect;
	bool				mKinectReady;
	bool				mKinectIR;
	Capture				mCap;
	gl::Texture			mTexture1, mTexture2;
	gl::Texture			mColorTexture, mDepthTexture;
	Surface8u			mDepthSurface;
	Surface8u			mColorSurface;
	
	gl::Texture			mNearThreshTexture, mFarThreshTexture, mCombinedTexture;
	
	SilhouetteDetector *silhouetteDetector;
	vector< vector<cv::Point> > * contours;
	params::InterfaceGl	mParams;
};

void ContoursApp::prepareSettings( Settings *settings ) {
	settings->setWindowSize( 1280, 800 );
	settings->setFrameRate( 60.0f );	
}

void ContoursApp::setup()
{	
	mKinect = Kinect( Kinect::Device() );
	mKinectReady = false;
	mKinectIR = false;
	
	silhouetteDetector = new SilhouetteDetector(640,480);
	contours = new vector<vector<cv::Point> >();
	
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 100 ) );
	mParams.addParam( "Near Threshold", &silhouetteDetector->mNearThreshold, "min=0 max=255 step=1.0 keyIncr=z keyDecr=Z" );
	mParams.addParam( "Far Threshold", &silhouetteDetector->mFarThreshold, "min=0 max=255 step=1.0 keyIncr=x keyDecr=X" );
	mParams.addParam( "Blur Amount", &silhouetteDetector->mBlurAmt, "min=0 max=20 step=1.0 keyIncr=b keyDecr=B" );
}

void ContoursApp::mouseDown( MouseEvent event )
{
}

void ContoursApp::update()
{
	if ( mKinectReady && !mKinectIR )
		mKinect.setVideoInfrared( true );
	
	if( mKinect.checkNewDepthFrame() ) {
		mDepthTexture = mKinect.getDepthImage();
		mDepthSurface = Surface8u( mKinect.getDepthImage() );
		mKinectReady = true;
		
		ci::Surface captureSurface = Surface8u( mKinect.getDepthImage() );
		ci::Surface outputSurface = captureSurface;
		
		contours->clear();
		
		silhouetteDetector->processSurface(&captureSurface, contours, &outputSurface);
		
		console() << contours->size() << " is the size " << endl;
		
		
		mTexture1 = outputSurface;
	}
	
	if( mKinect.checkNewColorFrame() ) {
		mTexture2 = gl::Texture( mKinect.getVideoImage() );
	}
}

void ContoursApp::draw()
{
	gl::clear( Color( CM_RGB, .5f, .5f, .5f ) );
	
	gl::color( Color( CM_RGB, 1.0f, 1.0f, 1.0f ) );
	
	if( mTexture1 )
		gl::draw( mTexture1 );
	
		
	
	gl::color( Color( CM_RGB, 0.0f, 1.0f, 1.0f ) );
	glLineWidth(3.0f);

	
	for ( vector<vector<cv::Point> >::iterator contour = contours->begin(); contour != contours->end(); contour++ ) {
		glBegin( GL_LINE_STRIP );
		for ( vector<cv::Point>::iterator point = (*contour).begin(); point != (*contour).end(); point++) {
			glVertex2f( (*point).x, (*point).y );
		}
		glEnd();
	}
	
	if ( mTexture2 ) {
		gl::color( Color( CM_RGB, 1.0f, 1.0f, 1.0f ) );
		gl::draw( mTexture2, Rectf( 320, 240, 640, 480 ) );
	}
	
	params::InterfaceGl::draw();
}


CINDER_APP_BASIC( ContoursApp, RendererGl )
