
#ifndef _ZEQ_MICRO_TIMER_H
#define _ZEQ_MICRO_TIMER_H

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "types.h"

class MicroTimer
{
#ifdef WIN32
	//high-precision timer for win32 cribbed from the internet
private:
	double frequency;
	LARGE_INTEGER start_count;
	LARGE_INTEGER end_count;

private:
	void pollCounter(LARGE_INTEGER& out)
	{
		HANDLE thread = GetCurrentThread();
		DWORD_PTR old_mask = SetThreadAffinityMask(thread, 0);
		QueryPerformanceCounter(&out);
		SetThreadAffinityMask(thread, old_mask);
	}

public:
	MicroTimer()
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		frequency = 1000000.0 / freq.QuadPart;
		start_count.QuadPart = 0;
		end_count.QuadPart = 0;
		pollCounter(start_count);
	}

	uint32 getElapsed()
	{
		pollCounter(end_count);
		double start_time = start_count.QuadPart * frequency;
		double end_time = end_count.QuadPart * frequency;
		return (uint32)(end_time - start_time);
	}
#else
private:
	uint32 start_time;

private:
	uint32 getTime()
	{
		timeval t;
		gettimeofday(&t, 0);
		return t.tv_sec * 1000000 + t.tv_usec;
	}

public:
	MicroTimer()
	{
		start_time = getTime();
	}
	uint32 getElapsed()
	{
		return getTime() - start_time;
	}
#endif
};

#endif

