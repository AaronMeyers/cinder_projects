
#include "GridMesh.h"

GridMesh::GridMesh() {}

void GridMesh::init(int xRes, int yRes, float xScale, float yScale, bool doColor, bool staticPositions) {
	mTriIndex = 0;
	mShader = gl::GlslProg( loadResource( "grid_vert.glsl" ), loadResource( "grid_frag.glsl" ) );
	
	mXRes = xRes;
	mYRes = yRes;
	
	mDoColor = doColor;
	
	mScale.set( xScale, yScale, 1 );
	
	int totalVertices	= xRes * yRes;
	int totalQuads		= (xRes-1) * (yRes-1);
	int totalTriangles	= totalQuads * 2;
	
	mPerlinScale = 500.0f;
	mDepthScale = 1000.0f;
	mDepthOffset = 300.0f;
	mShaderDisplacement = false;
	mDepthMax = 1.0;
	
	mStaticPositions = staticPositions;
	
	gl::VboMesh::Layout layout;
	layout.setStaticIndices();
	if (mStaticPositions)
		layout.setStaticPositions();
	else
		layout.setDynamicPositions();
	layout.setStaticTexCoords2d();
	layout.setDynamicNormals();
	if (mDoColor)
		layout.setDynamicColorsRGB();
	
	mVboMesh = gl::VboMesh( totalVertices, totalTriangles * 3, layout, GL_TRIANGLES);
	
	vector<Vec2f> texCoords;
	vector<Vec3f> positions;
	for( int x = 0; x < xRes; ++x ) {
		for( int y = 0; y < yRes; ++y ) {
			// create a quad for each vertex, except for along the bottom and right edges
			if( ( x + 1 < xRes ) && ( y + 1 < yRes ) ) {
				mIndices.push_back( (x+0) * yRes + (y+0) );
				mIndices.push_back( (x+1) * yRes + (y+0) );
				mIndices.push_back( (x+0) * yRes + (y+1) );
				
				mIndices.push_back( (x+1) * yRes + (y+0) );
				mIndices.push_back( (x+1) * yRes + (y+1) );
				mIndices.push_back( (x+0) * yRes + (y+1) );
			}
			// the texture coordinates are mapped to [0,1.0)
			texCoords.push_back( Vec2f( x / (float)xRes, y / (float)yRes ) );
			// create a vertex too
			mVertices.push_back(Vec3f::zero());
			// and a normal
			mNormals.push_back(Vec3f::zero());
			if ( mDoColor ) {
				// and a color
				mColors.push_back(Color());
			}
			
			if ( mStaticPositions ) {
				positions.push_back( Vec3f( (x / (float)mXRes) * mScale.x, (y / (float)mYRes) * mScale.y, 0 ) );
			}
		}
	}
	
	console() << "num vertices: " << mVertices.size() << std::endl;
	console() << "num indices: " << mIndices.size() << std::endl;
	
	if (mStaticPositions)
		mVboMesh.bufferPositions( positions );
	mVboMesh.bufferIndices( mIndices );
	mVboMesh.bufferTexCoords2d( 0, texCoords );
}

void GridMesh::update() {
	
	calcPositions();
	calcNormals();
}

void GridMesh::updateKinect(Kinect &kinect) {
	calcPositionsKinect(kinect);
	calcNormals();
}

void GridMesh::calcPositionsKinect(Kinect &kinect) {
	shared_ptr<uint16_t> depthData = kinect.getDepthData();
//	uint16_t col3Row2Depth = depthData.get()[2*640+3];
//	console() << col3Row2Depth << endl;
	
	
	gl::VboMesh::VertexIter iter = mVboMesh.mapVertexBuffer();
	for( int x = 0; x < mXRes; ++x ) {
		for( int y = 0; y < mYRes; ++y ) {
			Vec2f pos2d = Vec2f( x / (float)mXRes, y / (float)mYRes );
			uint16_t depthInt = depthData.get()[ ( (480 - (y+1) * ((int)mScale.y)/mYRes) * 640) + x * ((int)mScale.x)/mXRes ];
//			float depth = mShaderDisplacement ? 0 : (depthInt / (float)10000) * mDepthScale - mDepthOffset;
			float depth = mShaderDisplacement ? 0 : (depthInt / (float)65535) * mDepthScale - mDepthOffset;
			mVertices[ y + x * mYRes].set(pos2d.x * mScale.x, pos2d.y * mScale.y, depth);
			if (!mStaticPositions) {
				iter.setPosition( mVertices[ y + x * mYRes] );
				++iter;
			}
		}
	}
}

void GridMesh::calcPositions() {
	
	gl::VboMesh::VertexIter iter = mVboMesh.mapVertexBuffer();
	for( int x = 0; x < mXRes; ++x ) {
		for( int y = 0; y < mYRes; ++y ) {
			Vec2f pos2d = Vec2f( x / (float)mXRes, y / (float)mYRes );
			//			Vec3f pos = Vec3f( pos2d, mPerlin.fBm( Vec3f( pos2d, app::getElapsedSeconds() * 0.1f ) ) * mPerlinScale );
			mVertices[ y + x * mYRes].set(pos2d.x * mScale.x, pos2d.y * mScale.y, mPerlin.fBm( Vec3f( pos2d, app::getElapsedSeconds() * 0.1f ) ) * mPerlinScale);
//			mVertices[ y + x * mYRes].set(pos2d.x * mScale.x, pos2d.y * mScale.y, sin( pos2d.x * 3.14159f * 2.0f + getElapsedFrames()*.01f) * mPerlinScale);
			if (!mStaticPositions) {
				iter.setPosition( mVertices[ y + x * mYRes] );
				++iter;
			}
		}
	}
}

void GridMesh::drawNormals() {
	
	for( int x = 0; x < mXRes; x += 2 ) {
		for( int y = 0; y < mYRes; y += 2 ) {
			Vec3f position = mVertices[ x + y * mXRes ];
			Vec3f end = position + mNormals[ x + y * mYRes ] * 20.0f;
			gl::color( Color(CM_RGB, 1.0f, 0.0f, 0.0f) );
			gl::drawVector(position, end);
		}
	}
}

void GridMesh::calcNormals() {
	
	for (int i = 0; i < mNormals.size(); ++i) {
		mNormals[i].set(0,0,0);
	}
	
	
	// iterate through all the triangles
	int numTriangles = (mXRes-1) * (mYRes-1) * 2;
	for (int i = 0; i < numTriangles; ++i) {
		
		Vec3f &v0 = mVertices[mIndices[i * 3]];
		Vec3f &v1 = mVertices[mIndices[i * 3 + 1]];
		Vec3f &v2 = mVertices[mIndices[i * 3 + 2]];
		
		Vec3f c0 = v1 - v0;
		Vec3f c1 = v2 - v0;
		Vec3f n = c0.cross(c1).normalized();
		
//		console() << i << " --- " << n.x << ", " << n.y << ", " << n.z << std::endl;
//		console() << i << " v0:" << v0 << " v1:" << v1 << " v2:" << v2 << " c0:" << c0 << " c1:" << c1 << " n:" << n << std::endl;
		
		mNormals[mIndices[i * 3]] += n;
		mNormals[mIndices[i * 3 + 1]] += n;
		mNormals[mIndices[i * 3 + 2]] += n;
	}
	
	gl::VboMesh::VertexIter iter = mVboMesh.mapVertexBuffer();
	for( int x = 0; x < mXRes; ++x ) {
		for( int y = 0; y < mYRes; ++y ) {
			
			if ( mDoColor ) {
				mColors[ x + y * mXRes ].set(CM_HSV, Vec3f( x / (float)mXRes, y / (float)mYRes, 1.0f ) );
				iter.setColorRGB(mColors[ x + y * mXRes] );
			}
			
			Vec3f &normal = mNormals[ y + x * mYRes ];
//			normal.set(0, 0, 1);
			normal.normalize();
			iter.setNormal(normal);
			++iter;
		}
	}
}

void GridMesh::draw(float brightness) {
	
	mShader.bind();
	mShader.uniform( "NumEnabledLights", 1);
	mShader.uniform( "shaderDisplacement", mShaderDisplacement);
	mShader.uniform( "displacementMap", 0 );
	mShader.uniform( "normalMap", 1);
	mShader.uniform( "depthScale", mDepthScale );
	mShader.uniform( "depthOffset", mDepthOffset );
	mShader.uniform( "colorMap", 2);
	mShader.uniform( "depthMax", mDepthMax );
	
	gl::pushModelView();
	gl::translate( Vec3f(-mScale.x / 2.0f, -mScale.y / 2.0f, 0.0f) );
	gl::draw( mVboMesh );
	gl::popModelView();
	
	mShader.unbind();
}

Vec2f GridMesh::getDimensions() {
	return Vec2f(mScale.x, mScale.y);
}