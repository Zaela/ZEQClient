
#ifndef _ZEQ_TRANSLATE_H
#define _ZEQ_TRANSLATE_H

//static translation functions for id numbers, etc

#include <string>
#include <unordered_map>
#include "types.h"
#include "zeq_lua.h"

namespace Translate
{
	void initialize();

	int raceID(std::string race3letter);
	int gender(std::string race3letter);
	int animationID(std::string anim3letter);
	bool invertHeadingRace(int race);
}

#endif
