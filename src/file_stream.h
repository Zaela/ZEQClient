
#ifndef _ZEQ_FILE_STREAM_H
#define _ZEQ_FILE_STREAM_H

#include <cstdio>

#include "types.h"
#include "exception.h"
#include "memory_stream.h"

class FileStream : public MemoryStream
{
private:
	static void readFile(FILE* fp, byte*& data, uint32& len)
	{
		//not completely portable (good enough for Windows and Linux, at least), fix this sometime
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);

		if (len == 0)
		{
			fclose(fp);
			return;
		}

		rewind(fp);
		data = new byte[len];
		fread(data, 1, len, fp);
		fclose(fp);
	}

	FileStream(byte* data, uint32 len)
	{
		setData(data);
		setLen(len);
	}

public:
	FileStream(const char* path)
	{
		FILE* fp = fopen(path, "rb");
		if (fp == nullptr)
			throw ZEQException("FileStream::FileStream: Could not open file '%s'", path);

		byte* data = nullptr;
		uint32 len;
		readFile(fp, data, len);

		if (data == nullptr)
			throw ZEQException("FileStream::FileStream: Attempt to open empty file '%s'", path);

		setData(data);
		setLen(len);
	}

	//does not throw exceptions on failure
	static FileStream* open(const char* path)
	{
		FILE* fp = fopen(path, "rb");
		if (fp == nullptr)
			return nullptr;

		byte* data = nullptr;
		uint32 len;
		readFile(fp, data, len);

		if (data == nullptr)
			return nullptr;

		return new FileStream(data, len);
	}
};

#endif
