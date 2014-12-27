
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

	skele->setModelRaceGender(race_id, gender);

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

	mMobPositionList.push_back(MobPosition(
		Util::EQ19toFloat(spawn->y),
		Util::EQ19toFloat(spawn->z),
		Util::EQ19toFloat(spawn->x)
	)); //yzx - don't ask

	MobEntry ent;
	ent.entity_id = spawn->spawnId;
	ent.ptr = new Mob(spawn, proto->skeleton, &mMobPositionList.back(),
		proto->heads.size() ? proto->heads[0] : nullptr);

	mMobList.push_back(ent);
	printf("spawning race %i gender %i at %g, %g, %g - %s\n", spawn->race, spawn->gender, Util::EQ19toFloat(spawn->y),
		Util::EQ19toFloat(spawn->z), Util::EQ19toFloat(spawn->x), spawn->name);

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
				//inform mob of new indices
				mMobList[i].ptr->setIndex(i);
			}
			mMobList.pop_back();
			mMobPositionList.pop_back();
			return;
		}
	}
}

void MobManager::animateNearbyMobs(float delta)
{
	const MobPosition& pos = gPlayer.getCoords();

	const float dist = 1000.0f * 1000.0f;

	for (uint32 i = 0; i < mMobPositionList.size(); ++i)
	{
		if (Util::getDistSquared(pos, mMobPositionList[i]) <= dist)
			mMobList[i].ptr->animate(delta);
	}
}

Mob* MobManager::getMobByEntityID(int entity, bool& isPlayer)
{
	if (entity == gPlayer.getEntityID())
	{
		isPlayer = true;
		return nullptr;
	}
	isPlayer = false;

	for (MobEntry& mob : mMobList)
	{
		if (mob.entity_id == entity)
			return mob.ptr;
	}

	return nullptr;
}

void MobManager::handleHPUpdate(HPUpdate_Struct* update)
{
	bool isPlayer;
	Mob* mob = getMobByEntityID(update->spawn_id, isPlayer);

	if (mob)
		mob->setPercentHP(update->hp);
	else if (isPlayer)
		return; //handle
}

void MobManager::handleHPUpdate(ExactHPUpdate_Struct* update)
{
	bool isPlayer;
	Mob* mob = getMobByEntityID(update->spawn_id, isPlayer);

	if (mob)
	{
		mob->setExactHPMax(update->max_hp);
		mob->setExactHPCurrent(update->cur_hp);
	}
	else if (isPlayer)
	{
		//handle
	}
}

void MobManager::handlePositionUpdate(MobPositionUpdate_Struct* update)
{
	bool isPlayer;
	Mob* mob = getMobByEntityID(update->spawn_id, isPlayer);

	if (mob)
		mob->updatePosition(update);
	else if (isPlayer)
		return; //handle
}
