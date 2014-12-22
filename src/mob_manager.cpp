
#include "mob_manager.h"
#include "renderer.h"

extern Renderer gRenderer;

void MobManager::addModelPrototype(std::string model_id, WLDSkeleton* skele)
{
	MobPrototypeWLD proto;
	proto.skeleton = skele;
	mPrototypesWLD[model_id] = proto;
}

Mob* MobManager::addMob(std::string model_id)
{
	WLDSkeleton* skele = mPrototypesWLD[model_id].skeleton;

	MobPosition pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;

	mMobPositionList.push_back(pos);

	MobEntry ent;
	ent.entity_id = 0;
	ent.ptr = new Mob(mMobList.size(), gRenderer.addSkeletonInstance(skele), &mMobPositionList.back());

	mMobList.push_back(ent);

	return ent.ptr;
}
