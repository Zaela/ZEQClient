
#ifndef _ZEQ_MOB_H
#define _ZEQ_MOB_H

#include <irrlicht.h>

#include "types.h"
#include "wld_skeleton.h"
#include "structs_titanium.h"

struct MobPosition
{
	MobPosition() { }
	MobPosition(float inX, float inY, float inZ) :
		x(inX), y(inY), z(inZ) { }
	float x, y, z;
};

class Mob
{
private:
	uint32 mIndex;
	MobPosition* mPosition;
	scene::IAnimatedMeshSceneNode* mNode;
	WLDSkeletonInstance* mSkeletonWLD;
	WLDSkeletonInstance* mHeadSkeleWLD;

	int mEntityID;
	uint32 mExactCurHP;
	uint32 mExactMaxHP;
	uint8 mPercentHP;

public:
	Mob(uint32 index, WLDSkeleton* skele, MobPosition* pos, WLDSkeleton* head = nullptr);
	Mob(Spawn_Struct* spawn, WLDSkeleton* skele, MobPosition* pos);
	~Mob();

	void init(WLDSkeleton* skele, MobPosition* pos, WLDSkeleton* head = nullptr);

	void animate(float delta) { mSkeletonWLD->animate(delta); }
	void startAnimation(std::string id) { mSkeletonWLD->setAnimation(id); }

	void setExactHPCurrent(uint32 hp) { mExactCurHP = hp; }
	void setExactHPMax(uint32 hp) { mExactMaxHP = hp; }
	void setPercentHP(uint8 percent) { mPercentHP = percent; }
};

#endif
