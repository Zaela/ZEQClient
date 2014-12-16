
#ifndef _ZEQ_S3D_H
#define _ZEQ_S3D_H

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <regex>
#include <cctype>

#include "types.h"
#include "memory_stream.h"
#include "file_stream.h"
#include "buffer.h"
#include "compression.h"

class S3D
{
private:
	struct InternalFile
	{
		std::string name;
		uint32 offset;
		uint32 inflatedLen;
		uint32 deflatedLen;
		MemoryStream* decompressedFile;
	};

	MemoryStream* mRawData;
	std::vector<InternalFile> mFileArray;
	std::unordered_map<std::string, uint32> mFilePositionsByName;
	std::unordered_map<std::string, std::vector<uint32>> mFilePositionsByExt;

private:
	void open(MemoryStream* data);
	MemoryStream* decompressFile(InternalFile& file);

	bool extensionFileCheck(const char* ext, uint32 pos) const;

public:
	S3D(const char* path);
	S3D(MemoryStream* data);
	~S3D();

	MemoryStream* getFile(uint32 pos);
	MemoryStream* getFile(const char* name);
	MemoryStream* getFileByExtension(const char* ext, uint32 pos = 0);
	const char* getFileNameByExtension(const char* ext, uint32 pos = 0);

	uint32 getNumFilesWithExtension(const char* ext) const;
};

namespace S3D_Structs
{
	struct Header
	{
		uint32 offset;
		char magic[4];
		uint32 unknown;
	};

	struct BlockHeader
	{
		uint32 deflatedLen;
		uint32 inflatedLen;
	};

	struct DirEntry
	{
		uint32 crc;
		uint32 offset;
		uint32 inflatedLen;
	};
}

#endif
