
#include "mob_manager.h"
#include "renderer.h"
#include "player.h"

extern Renderer gRenderer;
extern Player gPlayer;

void MobManager::addModelPrototype(int race_id, int gender, WLDSkeleton* skele)
{
	MobPrototypeSetWLD& proto = mPrototypesWLD[race_id];

	if (proto.set[gender].skeleton) //don't overwrite
		return;

	proto.set[gender].skeleton = skele;
}

Mob* MobManager::spawnMob(int race_id, int gender, int level)
{
	//make sure we have the model, else default to human - add this later
	if (mPrototypesWLD.count(race_id) == 0 || mPrototypesWLD[race_id].set[gender].skeleton == nullptr)
		return nullptr;

	WLDSkeleton* skele = mPrototypesWLD[race_id].set[gender].skeleton;

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

Mob* MobManager::spawnMob(Spawn_Struct* spawn)
{
	return nullptr;
}

void MobManager::handleHPUpdate(HPUpdate_Struct* update)
{
	int entity = update->spawn_id;
	//if (entity == gPlayer.getEntityID())
	
	for (MobEntry& mob : mMobList)
	{
		if (mob.entity_id == entity)
		{
			mob.ptr->setPercentHP(update->hp);
			return;
		}
	}
}

void MobManager::handleHPUpdate(ExactHPUpdate_Struct* update)
{
	int entity = update->spawn_id;
	//if (entity == gPlayer.getEntityID())

	for (MobEntry& mob : mMobList)
	{
		if (mob.entity_id == entity)
		{
			mob.ptr->setExactHPMax(update->max_hp);
			mob.ptr->setExactHPCurrent(update->cur_hp);
			return;
		}
	}
}
