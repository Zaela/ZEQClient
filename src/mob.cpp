
#include "mob.h"
#include "renderer.h"

extern Renderer gRenderer;

Mob::Mob(uint32 index, WLDSkeletonInstance* skele, MobPosition* pos) :
	mIndex(index),
	mPosition(pos),
	mSkeletonWLD(skele)
{
	scene::IAnimatedMesh* mesh = new scene::SAnimatedMesh(skele->getMesh());

	mNode = gRenderer.getSceneManager()->addAnimatedMeshSceneNode(mesh, nullptr, -1,
		core::vector3df(pos->x, pos->y, pos->z));
	mesh->drop();
}
