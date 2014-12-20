
#ifndef _ZEQ_MOB_MODEL_H
#define _ZEQ_MOB_MODEL_H

#include <irrlicht.h>
#include <CSkinnedMesh.h>

#include <functional>
#include <vector>
#include <unordered_map>
#include <string>

#include "types.h"

using namespace irr;

struct Animation
{
	Animation()
	{
		frame_delay = 0;
		start_frame = 0;
		end_frame = 0;
	}

	uint32 frame_delay;
	uint32 start_frame;
	uint32 end_frame;
};

class MobModel
{
private:
	scene::ISkinnedMesh* mMesh;
	std::vector<scene::ISkinnedMesh*> mHeadMeshes;
	//change this to use animation ID numbers rather than strings, once mapping is known
	std::unordered_map<std::string, Animation, std::hash<std::string>> mAnimations;

public:
	MobModel();

	void setMesh(int n, scene::ISkinnedMesh* mesh);

	scene::ISkinnedMesh* getMainMesh() { return mMesh; }
	scene::ISkinnedMesh* getHeadMesh(uint32 n) { return mHeadMeshes[n]; }
	uint32 getNumHeadMeshes() { return mHeadMeshes.size(); }

	Animation* getAnimation(std::string id) { return &mAnimations[id]; }
};

#endif
