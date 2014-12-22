
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
public:
	uint32 mIndex;
	MobPosition* mPosition;
	scene::IAnimatedMeshSceneNode* mNode;
	WLDSkeletonInstance* mSkeletonWLD;

public:
	Mob(uint32 index, WLDSkeletonInstance* skele, MobPosition* pos);
};

#endif
