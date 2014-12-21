
#ifndef _ZEQ_WLD_SKELETON_H
#define _ZEQ_WLD_SKELETON_H

#include <irrlicht.h>

#include <vector>

#include "types.h"
#include "util.h"
#include "structs_wld.h"

using namespace irr;
using namespace WLD_Structs;

class WLDSkeleton
{
public:
	struct Animation
	{
		uint32 start_frame;
		uint32 end_frame;
		float frame_delay;
	};

	struct Weight
	{
		Weight(uint16 buf, uint16 vert) : buffer_index(buf), vert_index(vert) { }
		uint16 buffer_index;
		uint16 vert_index;
	};

private:
	//each frame has a complete skeleton, interpolate between current skeleton and next skeleton
	struct Frame
	{
		core::vector3df pos;
		core::vector3df rot;
	};

	struct FrameSkeleton
	{
		Frame* bones;
	};

	uint32 mNumBones;
	uint32 mNumFrames;
	FrameSkeleton mBasePosition;
	std::vector<FrameSkeleton> mFrames;
	std::vector<Weight>* mWeightsByBone;

private:
	static void getPosRot(Frag12Entry& f12, core::vector3df& pos, core::vector3df& rot);

public:
	WLDSkeleton(uint32 num_bones);

	uint32 getNumBones() { return mNumBones; }

	void addWeight(int bone, uint32 buffer, uint32 vert);
	std::vector<Weight>& getWeights(uint32 bone) { return mWeightsByBone[bone]; }

	void setBasePosition(int bone, Frag12Entry& f12, int parent = -1);
	void assumeBasePosition(scene::SMesh* mesh);
};

//the skeleton itself has no state, so it is fully reusable between instances of mobs
class WLDSkeletonInstance
{
public:
	scene::SMesh* mMesh;
	float mCurTime;
	uint32 mCurFrame;
	WLDSkeleton* mSkeleton;

public:
	WLDSkeletonInstance(scene::SMesh* mesh, WLDSkeleton* skele);

	void assumeBasePosition();
};

#endif
