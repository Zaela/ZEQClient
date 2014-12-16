
#ifndef _ZEQ_MEMORY_STREAM_H
#define _ZEQ_MEMORY_STREAM_H

#include "types.h"
#include "buffer.h"

class MemoryStream
{
private:
	byte* mData;
	size_t mLen;

protected:
	void setData(byte* data) { mData = data; }
	void setLen(size_t len) { mLen = len; }

public:
	MemoryStream() : mData(nullptr), mLen(0) { }
	MemoryStream(byte* data, size_t len) : mData(data), mLen(len) { }

	MemoryStream(Buffer* buf)
	{
		mData = buf->take();
		mLen = buf->length();
	}

	virtual ~MemoryStream() { if (mData) delete[] mData; }

	byte* getData() const { return mData; }
	size_t length() const { return mLen; }
};

#endif
