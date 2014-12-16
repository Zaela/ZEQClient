
#ifndef _ZEQ_ANIMATED_TEXTURE_H
#define _ZEQ_ANIMATED_TEXTURE_H

#include <irrlicht.h>

#include "types.h"
#include "structs_intermediate.h"

struct AnimatedTexture
{
public:
	const uint32 num_frames;
	const uint32 frame_delay;
	uint32 time;

private:
	uint32 cur_frame;
	const uint32 num_mesh_buffers;
	video::ITexture** texture_array;
	scene::IMeshBuffer** mesh_buffer_array;

public:
	AnimatedTexture(IntermediateMaterial* mat, uint32 buffer_count) :
		num_frames(mat->num_frames),
		frame_delay(mat->frame_delay),
		time(0),
		cur_frame(0),
		num_mesh_buffers(buffer_count)
	{
		texture_array = new video::ITexture*[num_frames];
		mesh_buffer_array = new scene::IMeshBuffer*[buffer_count];

		texture_array[0] = mat->first.diffuse_map;
		for (uint32 i = 1; i < num_frames; ++i)
			texture_array[i] = mat->additional[i - 1].diffuse_map;
	}

	void setMeshBuffer(uint32 i, scene::IMeshBuffer* buf)
	{
		if (i >= num_mesh_buffers)
			return;
		mesh_buffer_array[i] = buf;
	}

	void advanceFrame()
	{
		if (++cur_frame == num_frames)
			cur_frame = 0;
		video::ITexture* texture = texture_array[cur_frame];
		for (uint32 i = 0; i < num_mesh_buffers; ++i)
			mesh_buffer_array[i]->getMaterial().setTexture(0, texture);
			//mesh_buffer_array[i]->setDirty(); //might need this ?
	}
};

#endif
