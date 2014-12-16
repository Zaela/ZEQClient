
#include "util.h"

namespace Util
{
	void strcpy(char* dst, const char* src, uint32 boundLen)
	{
		--boundLen;
		uint32 count = 0;
		while (count < boundLen)
		{
			if (*src == 0)
			{
				*dst = 0;
				return;
			}
			*dst++ = *src++;
		}
		dst[boundLen] = 0;
	}
}
