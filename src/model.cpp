
#include "model.h"
#include "renderer.h"

extern Renderer gRenderer;

Model::~Model()
{
	for (AnimatedTexture& animTex : mAnimatedTextures)
		animTex.deleteArrays();

	for (video::ITexture* tex : mUsedTextures)
		gRenderer.destroyTexture(tex);
}

AnimatedTexture* Model::addAnimatedTexture(AnimatedTexture& animTex)
{
	//copy construction
	mAnimatedTextures.push_back(animTex);
	AnimatedTexture* anim = &mAnimatedTextures.back();
	//record textures
	anim->recordTextures(this);

	return anim;
}
