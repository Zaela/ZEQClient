
#ifndef _ZEQ_TER_H
#define _ZEQ_TER_H

#include <irrlicht.h>

#include <string>

#include "types.h"
#include "exception.h"
#include "memory_stream.h"
#include "s3d.h"
#include "structs_eqg.h"
#include "zone_model_source.h"
#include "zone_model.h"

using namespace EQG_Structs;

class TER : public ZoneModelSource
{
public:
	struct Header
	{
		char magic[4]; //EQGT
		uint32 version;
		uint32 strings_len;
		uint32 material_count;
		uint32 vertex_count;
		uint32 triangle_count;
	};

private:
	Header* mHeader;
	//const char* mStringBlock;
	//std::string mShortName;

public:
	TER(MemoryStream* mem, S3D* s3d, std::string shortname);

	//const char* getStringBlock() { return mStringBlock; }
	//std::string& getShortName() { return mShortName; }

	ZoneModel* convertZoneModel();
};


#endif
