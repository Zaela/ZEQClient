
#include "zone_model.h"

ZoneModel::ZoneModel() :
	mX(0), mY(0), mZ(0),
	mMesh(nullptr),
	mCollisionMesh(nullptr)
{

}

void ZoneModel::setMesh(scene::SMesh* mesh)
{
	mMesh = new scene::SAnimatedMesh(mesh);
	mesh->drop();
}

AnimatedTexture* ZoneModel::addAnimatedTexture(AnimatedTexture& animTex)
{
	//copy construction
	mAnimatedTextures.push_back(animTex);
	return &mAnimatedTextures.back();
}
