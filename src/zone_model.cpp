
#include "zone_model.h"
#include "renderer.h"

extern Renderer gRenderer;

ZoneModel::ZoneModel() :
	mX(0), mY(0), mZ(0),
	mMesh(nullptr),
	mNonCollisionMesh(nullptr)
{

}

ZoneModel::~ZoneModel()
{
	if (mMesh)
		mMesh->drop();
	if (mNonCollisionMesh)
		mNonCollisionMesh->drop();

	for (AnimatedTexture& animTex : mAnimatedTextures)
		animTex.deleteArrays();

	for (auto& pair : mObjectDefinitions)
		pair.second->drop();

	for (video::ITexture* tex : mUsedTextures)
		gRenderer.destroyTexture(tex);
}

void ZoneModel::setMeshes(scene::SMesh* mesh, scene::SMesh* nocollide_mesh)
{
	mMesh = new scene::SAnimatedMesh(mesh);
	mesh->drop();
	mNonCollisionMesh = new scene::SAnimatedMesh(nocollide_mesh);
	nocollide_mesh->drop();
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

void ZoneModel::addNoCollisionObjectDefinition(const char* name, scene::SMesh* mesh)
{
	scene::SAnimatedMesh* animMesh = new scene::SAnimatedMesh(mesh);
	mesh->drop();
	mNoCollisionObjectDefinitions[name] = animMesh;
}

void ZoneModel::addObjectPlacement(const char* name, ObjectPlacement& placement)
{
	if (mObjectDefinitions.count(name) > 0)
	{
		scene::IAnimatedMesh* mesh = mObjectDefinitions[name];
		placement.mesh = mesh;

		mObjectPlacements.push_back(placement);
	}

	if (mNoCollisionObjectDefinitions.count(name) > 0)
	{
		scene::IAnimatedMesh* mesh = mNoCollisionObjectDefinitions[name];
		placement.mesh = mesh;

		mObjectPlacements.push_back(placement);
	}
}
