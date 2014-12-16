
#ifndef _ZEQ_UTIL_H
#define _ZEQ_UTIL_H

#include "types.h"

namespace Util
{
	//strcpy with proper bounding
	void strcpy(char* dst, const char* src, uint32 boundLen);
}

#endif
