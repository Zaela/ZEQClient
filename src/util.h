
#ifndef _ZEQ_UTIL_H
#define _ZEQ_UTIL_H

#include <irrlicht.h>

#include <cctype>
#include <cmath>

#include "types.h"
#include "structs_mob.h"

using namespace irr;

namespace Util
{
	//strcpy with proper bounding
	void strcpy(char* dst, const char* src, uint32 boundLen);
	void toLower(char* str, uint32 len);

	float radiansToDegrees(float rad);
	void rotateBy(core::vector3df& pos, core::vector3df& rot);
	float getHeadingTo(const MobPosition& from, const MobPosition& to);

	float getDistSquared(const MobPosition& pos, const MobPosition& compare);

	float EQ19toFloat(int val);
	int floatToEQ19(float val);
	float EQ13toFloat(int val);
	int floatToEQ13(float val);
	float EQ13PreciseToFloat(int val);
	int floatToEQ13Precise(float val);
}

#endif
