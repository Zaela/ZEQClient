
#include "mob_manager.h"
#include "renderer.h"
#include "player.h"

extern Renderer gRenderer;
extern Player gPlayer;

bool MobManager::modelPrototypeLoaded(int race_id, int gender)
{
	return !(mPrototypesWLD.count(race_id) == 0 || mPrototypesWLD[race_id].set[gender].skeleton == nullptr);
}

void MobManager::addModelPrototype(int race_id, int gender, WLDSkeleton* skele, bool head)
{
	MobPrototypeSetWLD& proto = mPrototypesWLD[race_id];

	if (!head)
	{
		if (proto.set[gender].skeleton) //don't overwrite
			return;

		proto.set[gender].skeleton = skele;
		printf("added race %i gender %i\n", race_id, gender);
	}
	else
	{
		proto.set[gender].heads.push_back(skele);
		printf("added head for race %i gender %i\n", race_id, gender);
	}
}

MobPrototypeWLD* MobManager::getModelPrototype(int race_id, int gender)
{
	if (mPrototypesWLD.count(race_id) == 0 || mPrototypesWLD[race_id].set[gender].skeleton == nullptr)
	{
		return &mPrototypesWLD[DEFAULT_RACE].set[DEFAULT_GENDER];
	}

	return &mPrototypesWLD[race_id].set[gender];
}

Mob* MobManager::spawnMob(int race_id, int gender, int level, float x, float y, float z)
{
	MobPrototypeWLD* proto = getModelPrototype(race_id, gender);

	mMobPositionList.push_back(MobPosition(x, y, z));

	MobEntry ent;
	ent.entity_id = 0;
	ent.ptr = new Mob(mMobList.size(), proto->skeleton, &mMobPositionList.back(),
		proto->heads.size() ? proto->heads[0] : nullptr);

	mMobList.push_back(ent);

	return ent.ptr;
}

Mob* MobManager::spawnMob(Spawn_Struct* spawn)
{
	MobPrototypeWLD* proto = getModelPrototype(spawn->race, spawn->gender);

	mMobPositionList.push_back(MobPosition((float)spawn->x, (float)spawn->z, (float)spawn->y));

	MobEntry ent;
	ent.entity_id = spawn->spawnId;
	ent.ptr = new Mob(spawn->spawnId, proto->skeleton, &mMobPositionList.back());

	mMobList.push_back(ent);

	return ent.ptr;
}

void MobManager::despawnMob(int entity_id)
{
	for (uint32 i = 0; i < mMobList.size(); ++i)
	{
		if (mMobList[i].entity_id == entity_id)
		{
			delete mMobList[i].ptr;
			//we don't need to worry about re-entrance, the client makes no assumptions about connections between mobs
			if (i < (mMobList.size() - 1))
			{
				//swap'n'pop
				mMobList[i] = mMobList.back();
				//position list too
				mMobPositionList[i] = mMobPositionList.back();
				//inform mob of new positions
				mMobList[i].ptr->setIndex(i);
				mMobList[i].ptr->setPositionPtr(&mMobPositionList[i]);
			}
			mMobList.pop_back();
			mMobPositionList.pop_back();
			return;
		}
	}
}

void MobManager::animateNearbyMobs(float delta)
{
	MobPosition pos;
	gPlayer.getCoords(pos);

	const float dist = 1000.0f * 1000.0f;

	for (uint32 i = 0; i < mMobPositionList.size(); ++i)
	{
		if (Util::getDistSquared(pos, mMobPositionList[i]) <= dist)
			mMobList[i].ptr->animate(delta);
	}
}

void MobManager::handleHPUpdate(HPUpdate_Struct* update)
{
	int entity = update->spawn_id;
	if (entity == gPlayer.getEntityID())
	{
		//handle
		return;
	}
	
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
	if (entity == gPlayer.getEntityID())
	{
		//handle
		return;
	}

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

void MobManager::handlePositionUpdate(MobPositionUpdate_Struct* update)
{
	int entity = update->spawn_id;
	if (entity == gPlayer.getEntityID())
	{
		//handle
		return;
	}

	for (MobEntry& mob : mMobList)
	{
		if (mob.entity_id == entity)
		{
			mob.ptr->updatePosition(update);
			return;
		}
	}
}
