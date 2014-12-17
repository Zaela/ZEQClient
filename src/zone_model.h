
#ifndef _ZEQ_ZONE_MODEL_H
#define _ZEQ_ZONE_MODEL_H

#include <irrlicht.h>

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "types.h"
#include "animated_texture.h"

using namespace irr;

struct ObjectPlacement
{
	scene::IAnimatedMesh* mesh;
	float x, y, z;
	float rotX, rotY, rotZ;
	float scaleX, scaleY, scaleZ;
};

class ZoneModel
{
private:
	float mX, mY, mZ;
	scene::IAnimatedMesh* mMesh;
	scene::IAnimatedMesh* mCollisionMesh;
	std::vector<AnimatedTexture> mAnimatedTextures;
	std::unordered_map<std::string, scene::IAnimatedMesh*, std::hash<std::string>> mObjectDefinitions;
	std::vector<ObjectPlacement> mObjectPlacements;
	std::unordered_set<video::ITexture*> mUsedTextures;

public:
	ZoneModel();

	float getX() { return mX; }
	float getY() { return mY; }
	float getZ() { return mZ; }
	void setPosition(float x, float y, float z) { mX = x; mY = y; mZ = z; }

	void setMesh(scene::SMesh* mesh);
	scene::IAnimatedMesh* getMesh() { return mMesh; }
	AnimatedTexture* addAnimatedTexture(AnimatedTexture& animTex);
	const std::vector<AnimatedTexture>& getAnimatedTextures() { return mAnimatedTextures; }

	void addUsedTexture(video::ITexture* texture) { mUsedTextures.insert(texture); }
	void addObjectDefinition(const char* name, scene::SMesh* mesh);
	void addObjectPlacement(const char* name, ObjectPlacement& placement);
	const std::vector<ObjectPlacement>& getObjectPlacements() { return mObjectPlacements; }
};

#endif
