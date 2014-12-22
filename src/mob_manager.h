
#ifndef _ZEQ_MOB_MANAGER_H
#define _ZEQ_MOB_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>

#include "types.h"
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
	std::vector<MobEntry> mMobList;
	std::vector<MobPosition> mMobPositionList; //kept separate for faster computation on the whole set

	std::unordered_map<int, MobPrototypeSetWLD> mPrototypesWLD;

public:
	void addModelPrototype(int race_id, int gender, WLDSkeleton* skele);
	Mob* spawnMob(int race_id, int gender, int level = 1);
	Mob* spawnMob(Spawn_Struct* spawn);

	void handleHPUpdate(HPUpdate_Struct* update);
	void handleHPUpdate(ExactHPUpdate_Struct* update);
};

#endif
