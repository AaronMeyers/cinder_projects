#pragma once
#include "cinder/app/AppBasic.h"
#include "cinder/Vector.h"
#include "cinder/Utilities.h"
#include "cinder/Surface.h"
#include "CinderOpenCV.h"
#include <vector>

using namespace std;

class SilhouetteDetector {
public:
	SilhouetteDetector(int width, int height);
	~SilhouetteDetector();
	void processSurface(ci::Surface8u *captureSurface, vector< vector<cv::Point> > *contours, ci::Surface8u *processedOutput);
	int mNearThreshold, mFarThreshold;
	int mBlurAmt;
	
private:
	cv::Mat input;
	cv::Mat grayImage, grayThresh, grayThreshFar;
};