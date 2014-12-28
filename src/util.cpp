
#include "util.h"
#include "mob_manager.h"

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

	float getHeadingTo(const MobPosition& from, const MobPosition& to)
	{
		core::vector3df v(to - from);
		return v.getHorizontalAngle().Y;
	}

	float getDistSquared(const MobPosition& a, const MobPosition& b)
	{
		float x = a.X - b.X;
		float y = a.Y - b.Y;
		float z = a.Z - b.Z;

		return x*x + y*y + z*z;
	}

	float EQ19toFloat(int val)
	{
		return float(val) / float(1 << 3);
	}

	int floatToEQ19(float val)
	{
		return int(val * float(1 << 3));
	}

	float EQ13toFloat(int val)
	{
		return float(val) / float(1 << 2);
	}

	int floatToEQ13(float val)
	{
		return int(val * float(1 << 2));
	}

	float EQ13PreciseToFloat(int val)
	{
		return float(val) / float(1 << 6);
	}

	int floatToEQ13Precise(float val)
	{
		return int(val * float(1 << 6));
	}

	float unpackHeading(int eq19heading)
	{
		return EQ19toFloat(eq19heading) / 256.0f * 360.0f;
	}

	std::string getDisplayName(std::string name)
	{
		//are numeric symbols legal if not at the end?
		char buf[64];
		uint32 i;
		for (i = 0; i < name.length(); ++i)
		{
			char c = name[i];
			if (isdigit(c))
			{
				buf[i] = 0;
				break;
			}
			
			if (c == '_')
				buf[i] = ' ';
			else
				buf[i] = c;
		}

		buf[63] = 0;
		return std::string(buf, i);
	}
}
