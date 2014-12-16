
#ifndef _ZEQ_ZONE_MODEL_H
#define _ZEQ_ZONE_MODEL_H

#include <irrlicht.h>

#include <vector>

#include "types.h"
#include "animated_texture.h"

using namespace irr;

class ZoneModel
{
private:
	float mX, mY, mZ;
	scene::IAnimatedMesh* mMesh;
	scene::IAnimatedMesh* mCollisionMesh;
	std::vector<AnimatedTexture> mAnimatedTextures;

public:
	ZoneModel();

	float getX() { return mX; }
	float getY() { return mY; }
	float getZ() { return mZ; }
	void setPosition(float x, float y, float z) { mX = x; mY = y; mZ = z; }

	void setMesh(scene::SMesh* mesh);
	scene::IAnimatedMesh* getMesh() { return mMesh; }
	AnimatedTexture* addAnimatedTexture(AnimatedTexture& animTex);
};

#endif
