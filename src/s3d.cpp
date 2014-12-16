
#include "s3d.h"

using namespace S3D_Structs;

S3D::S3D(const char* path) : mRawData(nullptr)
{
	FileStream* fs = new FileStream(path);
	open(fs);
}

S3D::S3D(MemoryStream* ms) : mRawData(nullptr)
{
	open(ms);
}

S3D::~S3D()
{
	for (InternalFile& file : mFileArray)
	{
		if (file.decompressedFile)
			delete file.decompressedFile;
	}

	if (mRawData)
		delete mRawData;
}

void S3D::open(MemoryStream* mem_stream)
{
	mRawData = mem_stream;
	byte* data = mem_stream->getData();
	const size_t data_len = mem_stream->length();

	if (data_len < sizeof(Header))
		throw ZEQException("S3D::open: File was smaller than expected");

	Header* header = (Header*)data;
	const char* m = header->magic;
	if (m[0] != 'P' || m[1] != 'F' || m[2] != 'S' || m[3] != ' ')
		throw ZEQException("S3D:open: File was not a valid S3D");

	size_t p = header->offset;
	uint32 num_entries = *(uint32*)&data[p];
	p += sizeof(uint32);

	for (uint32 i = 0; i < num_entries; ++i)
	{
		DirEntry* dir_ent = (DirEntry*)&data[p];
		p += sizeof(DirEntry);

		InternalFile ent;
		ent.offset = dir_ent->offset;
		ent.inflatedLen = dir_ent->inflatedLen;

		uint32 dpos = ent.offset;
		uint32 ilen = 0;
		while (ilen < ent.inflatedLen)
		{
			BlockHeader* bh = (BlockHeader*)&data[dpos];
			dpos += sizeof(BlockHeader) + bh->deflatedLen;
			ilen += bh->inflatedLen;
		}

		ent.deflatedLen = dpos - ent.offset;
		ent.decompressedFile = nullptr;
		mFileArray.push_back(ent);
	}

	//dir entries are in order of crc, but names are in order of offset - need to sort
	auto offset_sort = [](InternalFile& a, InternalFile& b) -> bool
	{
		return a.offset < b.offset;
	};
	
	std::sort(mFileArray.begin(), mFileArray.end(), offset_sort);

	MemoryStream* name_data_stream = decompressFile(mFileArray.back());
	byte* name_data = name_data_stream->getData();
	mFileArray.pop_back(); //name data entry is not a real file
	num_entries = *(uint32*)name_data;
	p = sizeof(uint32);

	std::regex ext_regex("[^\\.]+$");
	std::cmatch match;

	for (uint32 i = 0; i < num_entries; ++i)
	{
		uint32 len = *(uint32*)&name_data[p];
		p += sizeof(uint32);
		char* name = (char*)&name_data[p];
		p += len;

		std::string file_name(name, len - 1); //len includes null terminator

		mFileArray[i].name = file_name;
		mFilePositionsByName[file_name] = i;

		if (std::regex_search(name, match, ext_regex))
		{
			std::string ext = match.str();
			mFilePositionsByExt[ext].push_back(i);
		}
	}

	delete name_data_stream;
}

MemoryStream* S3D::decompressFile(InternalFile& file)
{
	//static const uint32 BUFFER_SIZE = 16384;
	//static byte buffer[BUFFER_SIZE];
	uint32 pos = 0;
	Buffer buf;
	byte* data = mRawData->getData() + file.offset;

	while (pos < file.deflatedLen)
	{
		BlockHeader* bh = (BlockHeader*)&data[pos];
		pos += sizeof(BlockHeader);

		byte* decompressed = &data[pos];
		uint32 len = bh->deflatedLen;
		Compression::decompressBlock(decompressed, len);
		//unsigned long out_size = BUFFER_SIZE;
		//uncompress(buffer, &out_size, &data[pos], bh->deflatedLen);

		pos += bh->deflatedLen;

		buf.add(decompressed, bh->inflatedLen);
	}

	return new MemoryStream(&buf);
}

MemoryStream* S3D::getFile(uint32 pos)
{
	if (pos >= mFileArray.size())
		throw ZEQException("S3D:getFile: Out of range vector access");

	InternalFile& file = mFileArray[pos];
	if (file.decompressedFile)
		return file.decompressedFile;

	file.decompressedFile = decompressFile(file);
	return file.decompressedFile;
}

MemoryStream* S3D::getFile(const char* name)
{
	if (mFilePositionsByName.count(name) == 0)
		return nullptr;

	uint32 pos = mFilePositionsByName[name];
	return getFile(pos);
}

bool S3D::extensionFileCheck(const char* ext, uint32 pos) const
{
	if (mFilePositionsByExt.count(ext) == 0)
		return false;

	if (pos >= mFilePositionsByExt.at(ext).size())
		return false;

	return true;
}

MemoryStream* S3D::getFileByExtension(const char* ext, uint32 pos)
{
	if (!extensionFileCheck(ext, pos))
		return nullptr;

	return getFile(mFilePositionsByExt[ext][pos]);
}

const char* S3D::getFileNameByExtension(const char* ext, uint32 pos)
{
	if (!extensionFileCheck(ext, pos))
		return nullptr;

	return mFileArray[mFilePositionsByExt[ext][pos]].name.c_str();
}

uint32 S3D::getNumFilesWithExtension(const char* ext) const
{
	if (mFilePositionsByExt.count(ext) == 0)
		return 0;
	return mFilePositionsByExt.at(ext).size();
}
