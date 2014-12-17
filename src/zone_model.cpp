
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
	AnimatedTexture* anim = &mAnimatedTextures.back();
	//record textures
	anim->recordTextures(this);

	return anim;
}

void ZoneModel::addObjectDefinition(const char* name, scene::SMesh* mesh)
{
	scene::SAnimatedMesh* animMesh = new scene::SAnimatedMesh(mesh);
	mesh->drop();
	mObjectDefinitions[name] = animMesh;
}

void ZoneModel::addObjectPlacement(const char* name, ObjectPlacement& placement)
{
	if (mObjectDefinitions.count(name) == 0)
		return;

	scene::IAnimatedMesh* mesh = mObjectDefinitions[name];
	placement.mesh = mesh;

	mObjectPlacements.push_back(placement);
}
