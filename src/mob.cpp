
#include "mob.h"
#include "renderer.h"

extern Renderer gRenderer;

Mob::Mob(uint32 index, WLDSkeleton* skele, MobPosition* pos, WLDSkeleton* head) :
	mIndex(index)
{
	init(skele, pos, head);
}

Mob::Mob(Spawn_Struct* spawn, WLDSkeleton* skele, MobPosition* pos) :
	mIndex(spawn->spawnId)
{
	init(skele, pos);
}

void Mob::init(WLDSkeleton* skele, MobPosition* pos, WLDSkeleton* head)
{
	mPosition = pos;
	mSkeletonWLD = new WLDSkeletonInstance(gRenderer.copyMesh(skele->getReferenceMesh()), skele);
	mSkeletonWLD->assumeBasePosition();

	scene::IAnimatedMesh* mesh = new scene::SAnimatedMesh(mSkeletonWLD->getMesh());

	mNode = gRenderer.getSceneManager()->addAnimatedMeshSceneNode(mesh, nullptr, -1,
		core::vector3df(pos->x, pos->y, pos->z));
	mesh->drop();

	if (head)
	{
		mHeadSkeleWLD = new WLDSkeletonInstance(gRenderer.copyMesh(head->getReferenceMesh()), head);
		mHeadSkeleWLD->assumeBasePosition();

		mesh = new scene::SAnimatedMesh(mHeadSkeleWLD->getMesh());

		gRenderer.getSceneManager()->addAnimatedMeshSceneNode(mesh, mNode);
		mesh->drop();
	}
	else
	{
		mHeadSkeleWLD = nullptr;
	}

}

Mob::~Mob()
{
	if (mSkeletonWLD)
		delete mSkeletonWLD;
	mNode->drop();
}
