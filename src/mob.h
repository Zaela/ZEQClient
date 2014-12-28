
#ifndef _ZEQ_MOB_H
#define _ZEQ_MOB_H

#include <irrlicht.h>

#include "types.h"
#include "wld_skeleton.h"
#include "structs_mob.h"
#include "structs_titanium.h"
#include "translate.h"

class Mob
{
private:
	uint32 mIndex;
	scene::IAnimatedMeshSceneNode* mNode;
	WLDSkeletonInstance* mSkeletonWLD;
	WLDSkeletonInstance* mHeadSkeleWLD;

	scene::ISceneNode* mHeadNode;
	scene::ISceneNode* mRightHandNode;
	scene::ISceneNode* mLeftHandNode;
	scene::ISceneNode* mShieldNode;

	int mEntityID;
	char mDisplayName[64];
	char mRawName[64];

	//boolean block
	unsigned mInvertHeadingRace:1;
	//end boolean block

	uint32 mExactCurHP;
	uint32 mExactMaxHP;
	uint8 mPercentHP;

public:
	Mob(uint32 index, WLDSkeleton* skele, MobPosition* pos, WLDSkeleton* head = nullptr);
	Mob(Spawn_Struct* spawn, WLDSkeleton* skele, MobPosition* pos, WLDSkeleton* head);
	~Mob();

	void init(WLDSkeleton* skele, MobPosition* pos, WLDSkeleton* head = nullptr);

	void setHeading(float heading);

	void animate(float delta);
	void startAnimation(std::string id);

	void setIndex(uint32 i) { mIndex = i; }
	void setName(const char* name);
	void setExactHPCurrent(uint32 hp) { mExactCurHP = hp; }
	void setExactHPMax(uint32 hp) { mExactMaxHP = hp; }
	void setPercentHP(uint8 percent) { mPercentHP = percent; }
	void updatePosition(MobPositionUpdate_Struct* update);
};

#endif
