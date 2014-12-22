
#ifndef _ZEQ_MOB_H
#define _ZEQ_MOB_H

#include <irrlicht.h>

#include "types.h"
#include "wld_skeleton.h"

struct MobPosition
{
	float x, y, z;
};

class Mob
{
private:
	uint32 mIndex;
	MobPosition* mPosition;
	scene::IAnimatedMeshSceneNode* mNode;
	WLDSkeletonInstance* mSkeletonWLD;

	int mEntityID;
	uint32 mExactCurHP;
	uint32 mExactMaxHP;
	uint8 mPercentHP;

public:
	Mob(uint32 index, WLDSkeletonInstance* skele, MobPosition* pos);
	~Mob();

	void startAnimation(std::string id) { mSkeletonWLD->setAnimation(id); }

	void setExactHPCurrent(uint32 hp) { mExactCurHP = hp; }
	void setExactHPMax(uint32 hp) { mExactMaxHP = hp; }
	void setPercentHP(uint8 percent) { mPercentHP = percent; }
};

#endif
