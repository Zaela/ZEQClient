
#ifndef _ZEQ_MOB_MODEL_H
#define _ZEQ_MOB_MODEL_H

#include <irrlicht.h>

#include <vector>

#include "types.h"

using namespace irr;

class MobModel
{
private:
	scene::IAnimatedMesh* mMesh;
	std::vector<scene::IAnimatedMesh*> mHeadMeshes;

public:
	MobModel();

	void setMesh(int n, scene::SMesh* mesh);

	scene::IAnimatedMesh* getMainMesh() { return mMesh; }
	scene::IAnimatedMesh* getHeadMesh(uint32 n) { return mHeadMeshes[n]; }
	uint32 getNumHeadMeshes() { return mHeadMeshes.size(); }
};

#endif
