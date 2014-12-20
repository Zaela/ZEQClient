
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

	void rotateBy(core::vector3df& pos, core::vector3df& rot)
	{
		core::vector3df temp = pos;
		//x axis
		temp.Y = cos(rot.X) * pos.Y - sin(rot.X) * pos.Z;
		temp.Z = sin(rot.X) * pos.Y + cos(rot.X) * pos.Z;
		pos = temp;
		//y axis
		temp.X = cos(rot.Y) * pos.X + sin(rot.Y) * pos.Z;
		temp.Z = -sin(rot.Y) * pos.X + cos(rot.Y) * pos.Z;
		pos = temp;
		//z axis
		temp.X = cos(rot.Z) * pos.X - sin(rot.Z) * pos.Y;
		temp.Y = sin(rot.Z) * pos.X + cos(rot.Z) * pos.Y;
		pos = temp;
	}
}
