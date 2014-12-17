
#include "animated_texture.h"
#include "zone_model.h"

AnimatedTexture::AnimatedTexture(scene::SMesh* in_mesh, IntermediateMaterial* mat, uint32 index_count, uint32 index_base) :
	num_frames(mat->num_frames),
	frame_delay(mat->frame_delay),
	time(0),
	cur_frame(0),
	num_material_indices(index_count),
	mesh(in_mesh)
{
	texture_array = new video::ITexture*[num_frames];
	material_index_array = new uint32[index_count];
	for (uint32 i = 0; i < index_count; ++i)
		material_index_array[i] = index_base++;

	texture_array[0] = mat->first.diffuse_map;
	for (uint32 i = 1; i < num_frames; ++i)
		texture_array[i] = mat->additional[i - 1].diffuse_map;
}

void AnimatedTexture::advanceFrame()
{
	if (++cur_frame == num_frames)
		cur_frame = 0;
	video::ITexture* texture = texture_array[cur_frame];
	scene::IMeshSceneNode* node = (scene::IMeshSceneNode*)mesh;
	for (uint32 i = 0; i < num_material_indices; ++i)
		node->getMaterial(material_index_array[i]).setTexture(0, texture);
}

void AnimatedTexture::recordTextures(ZoneModel* zone)
{
	for (uint32 i = 0; i < num_frames; ++i)
		zone->addUsedTexture(texture_array[i]);
}

bool AnimatedTexture::replaceMeshWithSceneNode(void* compare_mesh, void* node)
{
	if (mesh == compare_mesh)
	{
		mesh = node;
		return true;
	}
	return false;
}
