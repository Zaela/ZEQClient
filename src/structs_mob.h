
#ifndef _ZEQ_STRUCTS_MOB_H
#define _ZEQ_STRUCTS_MOB_H

#include <irrlicht.h>

#include "types.h"

struct MobPosition
{
	MobPosition() { }
	MobPosition(float inX, float inY, float inZ)
	{
		set(inX, inY, inZ);
	}

	void set(float inX, float inY, float inZ)
	{
		x = inX;
		y = inY;
		z = inZ;
	}

	void set(core::vector3df& pos)
	{
		x = pos.X;
		y = pos.Y;
		z = pos.Z;
	}

	float x, y, z;
};

#endif
