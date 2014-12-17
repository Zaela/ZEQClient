
#ifndef _ZEQ_ANIMATED_TEXTURE_H
#define _ZEQ_ANIMATED_TEXTURE_H

#include <irrlicht.h>

#include "types.h"
#include "structs_intermediate.h"

class ZoneModel;

struct AnimatedTexture
{
public:
	const uint32 num_frames;
	const uint32 frame_delay;
	uint32 time;

private:
	uint32 cur_frame;
	const uint32 num_material_indices;
	video::ITexture** texture_array;
	uint32* material_index_array;
	void* mesh;

public:
	AnimatedTexture(scene::SMesh* mesh, IntermediateMaterial* mat, uint32 index_count, uint32 index_base);

	void advanceFrame();
	void recordTextures(ZoneModel* zone);
	bool replaceMeshWithSceneNode(void* compare_mesh, void* node);
};

#endif
