
#ifndef _ZEQ_FILE_STREAM_H
#define _ZEQ_FILE_STREAM_H

#include <cstdio>

#include "types.h"
#include "exception.h"
#include "memory_stream.h"

class FileStream : public MemoryStream
{
public:
	FileStream(const char* path)
	{
		FILE* fp = fopen(path, "rb");
		if (fp == nullptr)
			throw ZEQException("FileStream::FileStream: Could not open file '%s'", path);

		//not completely portable (good enough for Windows and Linux, at least), fix this sometime
		fseek(fp, 0, SEEK_END);
		size_t len = ftell(fp);

		if (len == 0)
		{
			fclose(fp);
			throw ZEQException("FileStream::FileStream: Attempt to open empty file '%s'", path);
		}

		rewind(fp);
		byte* data = new byte[len];
		fread(data, 1, len, fp);
		fclose(fp);

		setData(data);
		setLen(len);
	}
};

#endif
