
#ifndef _ZEQ_ANIMATED_TEXTURE_H
#define _ZEQ_ANIMATED_TEXTURE_H

#include <irrlicht.h>
#include <ISceneNodeAnimatorFactory.h>

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
	void* mesh; //need to change to reflect multipled separate scene nodes being made from same mesh...

public:
	AnimatedTexture(scene::SMesh* mesh, IntermediateMaterial* mat, uint32 index_count, uint32 index_base);

	void advanceFrame();
	void recordTextures(ZoneModel* zone);
	bool replaceMeshWithSceneNode(void* compare_mesh, void* node);
	bool checkMesh(void* compare_mesh) { return mesh == compare_mesh; }
	void setMeshPtr(void* ptr) { mesh = ptr; }
	void deleteArrays(); //don't want this to be in the destructor due to direct copies of the ptrs being made
};

#endif
