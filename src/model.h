
#ifndef _ZEQ_MODEL_H
#define _ZEQ_MODEL_H

#include <irrlicht.h>

#include <vector>
#include <unordered_set>

#include "types.h"
#include "animated_texture.h"

using namespace irr;

class Model
{
private:
	std::unordered_set<video::ITexture*> mUsedTextures;
	std::vector<AnimatedTexture> mAnimatedTextures;

public:
	virtual ~Model();

	void addUsedTexture(video::ITexture* texture) { mUsedTextures.insert(texture); }
	AnimatedTexture* addAnimatedTexture(AnimatedTexture& animTex);
	const std::vector<AnimatedTexture>& getAnimatedTextures() { return mAnimatedTextures; }
};

#endif
