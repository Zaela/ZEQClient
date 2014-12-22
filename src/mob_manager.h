
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

class MobManager
{
private:
	std::vector<MobEntry> mMobList;
	std::vector<MobPosition> mMobPositionList; //kept separate for faster computation on the whole set

	//change to race ids later
	std::unordered_map<std::string, MobPrototypeWLD, std::hash<std::string>> mPrototypesWLD;

public:
	void addModelPrototype(std::string model_id, WLDSkeleton* skele);
	Mob* addMob(std::string model_id);
};

#endif
