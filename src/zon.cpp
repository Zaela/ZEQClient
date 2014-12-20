
#include "zon.h"

ZON::ZON(MemoryStream* mem, S3D* s3d, std::string shortname, bool delete_me) :
	ModelSource(s3d, shortname),
	mTERPlacementIndex(-1),
	mRawData(mem),
	mDeleteMe(delete_me)
{
	byte* data = mem->getData();

	mHeader = (Header*)data;
	uint32 p = sizeof(Header);

	const char* m = mHeader->magic;
	if (m[0] != 'E' || m[1] != 'Q' || m[2] != 'G' || m[3] != 'Z')
		throw ZEQException("ZON::ZON: File was not a valid ZON");

	if (mHeader->version != 1) //version 2 zons are quite a bit different, only used for heightmapped zones (?)
		throw ZEQException("ZON::ZON: unsupported ZON version");

	mStringBlock = (char*)(data + p);
	Util::toLower(mStringBlock, mHeader->strings_len);
	p += mHeader->strings_len;

	//model names
	mModelNameIndices = (uint32*)(data + p);
	p += sizeof(uint32) * mHeader->model_count;

	//objects/placeables
	mObjectPlacements = (Object*)(data + p);
	p += sizeof(Object) * mHeader->object_count;

	//regions
	mRegions = (Region*)(data + p);
	p += sizeof(Region) * mHeader->region_count;

	//lights
	mLights = (Light*)(data + p);
}

ZON::~ZON()
{
	//zon file may come from the EQ folder directly rather than from an S3D, need to release file data in that case
	if (mDeleteMe && mRawData)
		delete mRawData;
}

TER* ZON::getTER()
{
	//it's usually the first one, so this should be quick
	uint32* ptr = mModelNameIndices;
	for (uint32 i = 0; i < mHeader->model_count; ++i)
	{
		char* name = &mStringBlock[*ptr++];
		size_t len = strlen(name);

		if (len < 5 || strcmp(name + len - 4, ".ter") != 0)
			continue;

		MemoryStream* file = mContainingS3D->getFile(name);
		if (file == nullptr)
			return nullptr;

		return new TER(file, mContainingS3D, mShortName);
	}

	return nullptr;
}
