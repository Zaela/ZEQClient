
#ifndef _ZEQ_WLD_SKELETON_H
#define _ZEQ_WLD_SKELETON_H

#include <irrlicht.h>

#include <string>
#include <vector>
#include <unordered_map>

#include "types.h"
#include "util.h"
#include "structs_wld.h"
#include "model.h"

using namespace irr;
using namespace WLD_Structs;

class Mob;

class WLDSkeleton : public Model
{
public:
	struct Weight
	{
		Weight(uint16 buf, uint16 vert) : buffer_index(buf), vert_index(vert) { }
		uint16 buffer_index;
		uint16 vert_index;
	};

	enum PointType
	{
		POINT_RIGHT = 1 << 0,
		POINT_LEFT = 1 << 1,
		POINT_SHIELD = 1 << 2,
		POINT_HEAD = 1 << 3
	};

	enum PointPosition
	{
		POINT_RIGHT_POS,
		POINT_LEFT_POS,
		POINT_SHIELD_POS,
		POINT_HEAD_POS,
		POINT_COUNT //not a real position
	};

private:
	//each frame has a complete skeleton, interpolate between current skeleton and next skeleton
	struct Frame
	{
		core::vector3df pos;
		core::vector3df rot;
	};

	struct BaseFrame
	{
		Frame bone;
		Frame* attachment_point;
	};

	struct FrameSkeleton
	{
		Frame* bones;
	};

	struct BaseSkeleton
	{
		BaseFrame* bones;
	};

	//for animations, the root bone and occasionally other bones will only have 1 frame;
	//attachment points have no frames at all, so they need to be handled specially as well
	//for all other bones, the number of frames should be the same as the number in the animation
	struct Animation
	{
		uint32 start_frame;
		uint32 end_frame;
		float frame_delay;
	};

	bool mIsCopy;
	uint32 mNumBones;
	uint32 mNumFrames;
	uint8 mHasPoint;
	BaseSkeleton mBasePosition;
	Frame mPoints[POINT_COUNT]; //basically a buffer that's only valid immediately after a mob is animated
	scene::SMesh* mReferenceMesh;
	std::vector<FrameSkeleton> mFrames; //could be changed to one big flat array of frames
	std::vector<Weight>* mWeightsByBone;
	std::unordered_map<std::string, Animation, std::hash<std::string>> mAnimations;

private:
	static void getPosRot(Frag12Entry& f12, core::vector3df& pos, core::vector3df& rot);
	static void inheritPosRot(core::vector3df& pos, core::vector3df& rot, Frame& parent);
	static void moveVertex(const video::S3DVertex& refVert, video::S3DVertex& vert, Frame& by);

public:
	WLDSkeleton(uint32 num_bones, scene::SMesh* reference_mesh);
	WLDSkeleton(const WLDSkeleton& toCopy); //copy constructor

	uint32 getNumBones() { return mNumBones; }
	void setReferenceMesh(scene::SMesh* mesh) { mReferenceMesh = mesh; }
	scene::SMesh* getReferenceMesh() { return mReferenceMesh; }

	void addWeight(int bone, uint32 buffer, uint32 vert);
	std::vector<Weight>& getWeights(uint32 bone) { return mWeightsByBone[bone]; }

	void setBasePosition(int bone, Frag12Entry& f12, int parent = -1, int point = -1);
	void assumeBasePosition(scene::SMesh* mesh);
	void addAnimation(std::string animName, uint32 num_frames, uint32 frame_delay);
	void addAnimationFrames(std::string animName, Frag12* f12, int bone_id, int parent_id = -1);
	uint32 animate(std::string animName, scene::SMesh* mesh, uint32 cur_frame, float& time, bool& ended);
};

//the skeleton itself has no state, so it is fully reusable between instances of mobs
class WLDSkeletonInstance
{
public:
	scene::SMesh* mMesh;
	float mCurTime;
	uint32 mCurFrame; //of the current animation, not absolute
	std::string mCurAnimation; //change to id number later
	WLDSkeleton* mSkeleton;
	Mob* mOwner;

public:
	WLDSkeletonInstance() { }
	WLDSkeletonInstance(scene::SMesh* mesh, WLDSkeleton* skele);

	scene::SMesh* getMesh() { return mMesh; }

	void assumeBasePosition();
	void setAnimation(std::string animName);
	void animate(float delta);
};

#endif
