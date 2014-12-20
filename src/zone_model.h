
#ifndef _ZEQ_ZONE_MODEL_H
#define _ZEQ_ZONE_MODEL_H

#include <irrlicht.h>

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "types.h"
#include "model.h"
#include "s3d.h"
#include "animated_texture.h"

using namespace irr;

class WLD;
class ZON;

struct ObjectPlacement
{
	scene::IAnimatedMesh* mesh;
	float x, y, z;
	float rotX, rotY, rotZ;
	float scaleX, scaleY, scaleZ;
	bool collidable;
};

class ZoneModel : public Model
{
private:
	float mX, mY, mZ;
	scene::IAnimatedMesh* mMesh;
	scene::IAnimatedMesh* mNonCollisionMesh;
	std::unordered_map<std::string, scene::IAnimatedMesh*, std::hash<std::string>> mObjectDefinitions;
	std::unordered_map<std::string, scene::IAnimatedMesh*, std::hash<std::string>> mNoCollisionObjectDefinitions;
	std::vector<ObjectPlacement> mObjectPlacements;

private:
	static ZoneModel* loadFromWLD(std::string shortname, WLD* wld);
	static ZoneModel* loadFromZON(std::string shortname, ZON* zon);

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

	void addObjectDefinition(const char* name, scene::SMesh* mesh);
	void addNoCollisionObjectDefinition(const char* name, scene::SMesh* mesh);
	void addObjectPlacement(const char* name, ObjectPlacement& placement);
	const std::vector<ObjectPlacement>& getObjectPlacements() { return mObjectPlacements; }

	static ZoneModel* load(std::string shortname);
};

#endif
