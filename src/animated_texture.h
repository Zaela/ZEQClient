
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
};


class AnimatedTextureFactory : public scene::ISceneNodeAnimatorFactory
{
public:
	virtual scene::ISceneNodeAnimator* createSceneNodeAnimator(scene::ESCENE_NODE_ANIMATOR_TYPE type, scene::ISceneNode* target) override;
	virtual scene::ISceneNodeAnimator* createSceneNodeAnimator(const char* typeName, scene::ISceneNode* target) override;
	virtual uint32 getCreatableSceneNodeAnimatorTypeCount() const override;
	virtual scene::ESCENE_NODE_ANIMATOR_TYPE getCreateableSceneNodeAnimatorType(uint32 idx) const override;
	virtual const char* getCreateableSceneNodeAnimatorTypeName(uint32 idx) const override;
	virtual const char* getCreateableSceneNodeAnimatorTypeName (scene::ESCENE_NODE_ANIMATOR_TYPE type) const override;
};

class TextureAnimator : public scene::ISceneNodeAnimator
{
public:
	TextureAnimator(AnimatedTexture& animTex);
	virtual void animateNode(scene::ISceneNode* node, uint32 milliseconds) override;
	virtual scene::ISceneNodeAnimator* createClone(scene::ISceneNode* node, scene::ISceneManager* newManager = nullptr) override;
};

#endif
