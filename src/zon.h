
#ifndef _ZEQ_ZON_H
#define _ZEQ_ZON_H

#include <string>

#include "types.h"
#include "util.h"
#include "exception.h"
#include "s3d.h"
#include "ter.h"
#include "mod.h"
#include "zone_model.h"
#include "model_source.h"
#include "memory_stream.h"

class ZON : public ModelSource
{
private:
	struct Header
	{
		char magic[4]; //EQGZ
		uint32 version;
		uint32 strings_len;
		uint32 model_count;
		uint32 object_count;
		uint32 region_count;
		uint32 light_count;
	};

	struct Object //"placeable"
	{
		int id;
		uint32 name_index;
		float x, y, z;
		float rotX, rotY, rotZ;
		float scale;
	};

	struct Region
	{
		uint32 name_index;
		float centerX, centerY, centerZ;
		float unknownA;
		uint32 unknownB, unknownC;
		float extentX, extentY, extentZ;
	};

	struct Light
	{
		uint32 name_index;
		float x, y, z;
		float r, b, g;
		float radius;
	};

	Header* mHeader;
	uint32* mModelNameIndices;
	Object* mObjectPlacements;
	Region* mRegions;
	Light* mLights;
	int mTERPlacementIndex; //to make sure we don't treat the TER as both a zone and a placement
	MemoryStream* mRawData;
	bool mDeleteMe;

public:
	ZON(MemoryStream* mem, S3D* s3d, std::string shortname, bool delete_me);
	~ZON();

	TER* getTER();
	void setZonePosition(ZoneModel* zoneModel);
	void readObjects(ZoneModel* zoneModel);
};

#endif
