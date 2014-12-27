
#ifndef _ZEQ_MOB_MANAGER_H
#define _ZEQ_MOB_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>

#include "types.h"
#include "util.h"
#include "mob.h"
#include "wld_skeleton.h"
#include "structs_titanium.h"

struct MobEntry
{
	int entity_id;
	Mob* ptr;
};

struct MobPrototypeWLD
{
	WLDSkeleton* skeleton;
	std::vector<WLDSkeleton*> heads;
};

struct MobPrototypeSetWLD
{
	MobPrototypeSetWLD()
	{
		for (int i = 0; i < 3; ++i)
			set[i].skeleton = nullptr;
	}

	MobPrototypeWLD set[3]; //0 = male, 1 = female, 2 = neuter
};

class MobManager
{
private:
	static const int DEFAULT_RACE = 1;
	static const int DEFAULT_GENDER = 0;

	std::vector<MobEntry> mMobList;
	std::vector<MobPosition> mMobPositionList; //kept separate for faster computation on the whole set

	std::unordered_map<int, MobPrototypeSetWLD> mPrototypesWLD;

private:
	MobPrototypeWLD* getModelPrototype(int race_id, int gender);

	Mob* getMobByEntityID(int entityid, bool& isPlayer);

public:
	bool modelPrototypeLoaded(int race_id, int gender);
	void addModelPrototype(int race_id, int gender, WLDSkeleton* skele, bool head = false);
	Mob* spawnMob(int race_id, int gender, int level = 1, float x = 0.0f, float y = 0.0f, float z = 0.0f);
	Mob* spawnMob(Spawn_Struct* spawn);
	void despawnMob(int entity_id);
	MobPosition* getMobPosition(uint32 index) { return &mMobPositionList[index]; }

	void animateNearbyMobs(float delta);

	void handleHPUpdate(HPUpdate_Struct* update);
	void handleHPUpdate(ExactHPUpdate_Struct* update);
	void handlePositionUpdate(MobPositionUpdate_Struct* update);
};

#endif
