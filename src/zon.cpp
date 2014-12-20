
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

		mTERPlacementIndex = i;
		MemoryStream* file = mContainingS3D->getFile(name);
		if (file == nullptr)
			return nullptr;

		return new TER(file, mContainingS3D, mShortName);
	}

	return nullptr;
}

void ZON::setZonePosition(ZoneModel* zoneModel)
{
	Object* obj = mObjectPlacements + mTERPlacementIndex;

	zoneModel->setPosition(obj->x, obj->z, obj->y);
}

void ZON::readObjects(ZoneModel* zoneModel)
{
	//find model definitions first
	uint32* ptr = mModelNameIndices;
	int n = mHeader->model_count;
	for (int i = 0; i < n; ++i, ++ptr)
	{
		if (i == mTERPlacementIndex)
			continue;

		char* name = &mStringBlock[*ptr];
		Util::toLower(name, strlen(name));

		MemoryStream* file = mContainingS3D->getFile(name);
		if (file == nullptr)
			continue;

		MOD mod(file, mContainingS3D, name);
		mod.convertStaticModel(zoneModel);
	}

	//now on to model placements
	for (uint32 i = 0; i < mHeader->object_count; ++i)
	{
		Object& obj = mObjectPlacements[i];
		const char* name = &mStringBlock[mModelNameIndices[obj.id]];

		ObjectPlacement op;
		op.x = obj.x;
		op.y = obj.z;
		op.z = obj.y;

		/*op.rotX = obj.rotX + 1.5708f; //zon subtracts 1.5708 from all x rotations for some reason, an orientation best known as completely sideways
		op.rotY = obj.rotZ;
		op.rotZ = obj.rotY;*/
		op.rotX = obj.rotX;
		op.rotY = obj.rotZ;
		op.rotZ = obj.rotY;

		op.scaleX = obj.scale;
		op.scaleY = obj.scale;
		op.scaleZ = obj.scale;

		zoneModel->addObjectPlacement(name, op);
	}
}
