
#ifndef _ZEQ_MOD_H
#define _ZEQ_MOD_H

#include <string>

#include "types.h"
#include "exception.h"
#include "s3d.h"
#include "model_source.h"
#include "memory_stream.h"
#include "structs_eqg.h"
#include "zone_model.h"

using namespace EQG_Structs;

class MOD : public ModelSource
{
private:
	struct Header
	{
		char magic[4]; //EQGM
		uint32 version;
		uint32 strings_len;
		uint32 material_count;
		uint32 vertex_count;
		uint32 triangle_count;
		uint32 bone_count;
	};

	Header* mHeader;

public:
	MOD(MemoryStream* mem, S3D* s3d, std::string shortname);

	//for future consideration: these may also be item models, and they may also be animated
	void convertStaticModel(ZoneModel* zoneModel);
};

#endif
