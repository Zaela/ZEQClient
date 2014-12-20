
#ifndef _ZEQ_UTIL_H
#define _ZEQ_UTIL_H

#include <irrlicht.h>

#include <cctype>
#include <cmath>

#include "types.h"

using namespace irr;

namespace Util
{
	//strcpy with proper bounding
	void strcpy(char* dst, const char* src, uint32 boundLen);
	void toLower(char* str, uint32 len);
	float radiansToDegrees(float rad);
	void rotateBy(core::vector3df& pos, core::vector3df& rot);
}

#endif
