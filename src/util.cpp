
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

	void toLower(char* str, uint32 len)
	{
		for (uint32 i = 0; i < len; ++i)
			str[i] = tolower(str[i]);
	}

	float radiansToDegrees(float rad)
	{
		return rad * 180.0f / 3.14159f;
	}
}
