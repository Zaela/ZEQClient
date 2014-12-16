
#ifndef _ZEQ_RANDOM_H
#define _ZEQ_RANDOM_H

#include <random>
#include <chrono>
#include <ctime>

#include "types.h"

class Random : public std::mt19937
{
public:
	Random()
	{
		time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		seed((uint32)time);
	}
};

#endif
