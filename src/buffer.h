
#ifndef _ZEQ_BUFFER_H
#define _ZEQ_BUFFER_H

#include <cstring>

#include "types.h"

class Buffer
{
private:
	byte* mData;
	size_t mLen;
	size_t mCap;

	static const size_t INITIAL_SIZE = 8192;

private:
	void resize(size_t newlen)
	{
		size_t cap = mCap;
		do
			cap <<= 1;
		while (cap <= newlen);

		byte* new_data = new byte[cap];
		memcpy(new_data, mData, mLen);
		delete[] mData;
		mCap = cap;
		mData = new_data;
	}

public:
	Buffer() : mLen(0), mCap(INITIAL_SIZE)
	{
		mData = new byte[INITIAL_SIZE];
	}

	~Buffer()
	{
		if (mData)
			delete[] mData;
	}

	void add(const void* in_data, size_t len)
	{
		const byte* data = (const byte*)in_data;
		size_t newlen = mLen + len;
		if (newlen >= mCap)
			resize(newlen);
		memcpy(mData + mLen, data, len);
		mLen = newlen;
	}

	byte* take()
	{
		byte* ret = mData;
		mData = nullptr;
		return ret;
	}

	size_t length() const { return mLen; }
};

#endif
