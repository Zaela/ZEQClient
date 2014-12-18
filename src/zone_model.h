
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
	scene::IAnimatedMesh* mNonCollisionMesh;
	std::vector<AnimatedTexture> mAnimatedTextures;
	std::unordered_map<std::string, scene::IAnimatedMesh*, std::hash<std::string>> mObjectDefinitions;
	std::unordered_map<std::string, scene::IAnimatedMesh*, std::hash<std::string>> mNoCollisionObjectDefinitions;
	std::vector<ObjectPlacement> mObjectPlacements;
	std::unordered_set<video::ITexture*> mUsedTextures;

public:
	ZoneModel();
	~ZoneModel();

	float getX() { return mX; }
	float getY() { return mY; }
	float getZ() { return mZ; }
	void setPosition(float x, float y, float z) { mX = x; mY = y; mZ = z; }

	void setMeshes(scene::SMesh* mesh, scene::SMesh* nocollide_mesh);
	scene::IAnimatedMesh* getMesh() { return mMesh; }
	scene::IAnimatedMesh* getNonCollisionMesh() { return mNonCollisionMesh; }
	AnimatedTexture* addAnimatedTexture(AnimatedTexture& animTex);
	const std::vector<AnimatedTexture>& getAnimatedTextures() { return mAnimatedTextures; }

	void addUsedTexture(video::ITexture* texture) { mUsedTextures.insert(texture); }
	void addObjectDefinition(const char* name, scene::SMesh* mesh);
	void addNoCollisionObjectDefinition(const char* name, scene::SMesh* mesh);
	void addObjectPlacement(const char* name, ObjectPlacement& placement);
	const std::vector<ObjectPlacement>& getObjectPlacements() { return mObjectPlacements; }
};

#endif
