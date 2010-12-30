
#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Perlin.h"
#include "cinder/gl/GlslProg.h"

using namespace ci;
using namespace ci::app;
using std::vector;

class GridMesh {
public:
	int					mXRes,
						mYRes;
	
	Perlin				mPerlin;
	float				mPerlinScale;
	Vec3f				mScale;
	gl::VboMesh			mVboMesh;
	
	vector<uint32_t>	mIndices;
	vector<Vec3f>		mVertices;
	vector<Vec3f>		mNormals;
	vector<Color>		mColors;
	
	bool				mDoColor;
	
	int mTriIndex;
	
	gl::GlslProg		mShader;
	
	GridMesh();
	void init(int xRes, int yRes, float xScale, float yScale, bool doColor = false);
	void update();
	void calcPositions();
	void calcNormals();
	void draw(float brightness = 1.0f);
	void drawNormals();
	
	Vec2f getDimensions();
};