#include "SilhouetteDetector.h"

using namespace std;
using namespace boost;
using namespace ci;
using namespace ci::app;

SilhouetteDetector::SilhouetteDetector(int width, int height) {	
	mNearThreshold = 127;
	mFarThreshold = 180;
	mBlurAmt = 2;
}

void SilhouetteDetector::processSurface(ci::Surface8u *captureSurface, vector< vector<cv::Point> > *contours, ci::Surface8u *processedOutput) {
	
	input = toOcv( *captureSurface );		//define cv::Mat -- CV Matrices that we'll need
	
	cv::cvtColor( input, grayImage, CV_RGB2GRAY );
	cv::medianBlur( grayImage, input, mBlurAmt * 2 + 1 );
	
	if (mFarThreshold <= mNearThreshold) mFarThreshold = mNearThreshold+1;
	
	cv::threshold( input, grayThresh, mNearThreshold, 255, CV_THRESH_BINARY );
	cv::threshold( input, grayThreshFar, mFarThreshold, 255, CV_THRESH_BINARY_INV );
	
	
	IplImage combinedIpl = grayImage;
	IplImage nearIpl = grayThresh;
	IplImage farIpl = grayThreshFar;
	
	cvAnd( &nearIpl, &farIpl, &combinedIpl, NULL);
	
	ci::Surface outputSurface = fromOcv(grayImage);
	processedOutput->setData(outputSurface.getData(),outputSurface.getWidth(),outputSurface.getHeight(),outputSurface.getRowBytes());
	
	if (contours != NULL) {
		cv::findContours( grayImage, *contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
	}
}

SilhouetteDetector::~SilhouetteDetector() {
}