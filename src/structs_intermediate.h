
#ifndef _ZEQ_STRUCTS_INTERMEDIATE_H
#define _ZEQ_STRUCTS_INTERMEDIATE_H

#include <irrlicht.h>

#include "types.h"

using namespace irr;

struct IntermediateMaterialEntry
{
	IntermediateMaterialEntry()
	{
		flag = 0;
		diffuse_map = nullptr;
		normal_map = nullptr;
	}

	enum Flags
	{
		MASKED = 1 << 0,
		SEMI_TRANSPARENT = 1 << 1,
		FULLY_TRANSPARENT = 1 << 2,
		DDS_TEXTURE = 1 << 3
	};

	uint32 flag;
	video::ITexture* diffuse_map;
	video::ITexture* normal_map;
};

struct IntermediateMaterial
{
	IntermediateMaterial()
	{
		num_frames = 1;
		additional = nullptr;
	}

	int num_frames; //if > 1, material is animated
	uint32 frame_delay;
	IntermediateMaterialEntry first;
	IntermediateMaterialEntry* additional;
};

#endif
