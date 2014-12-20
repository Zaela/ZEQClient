
#ifndef _ZEQ_ZONE_MODEL_SOURCE_H
#define _ZEQ_ZONE_MODEL_SOURCE_H

#include <irrlicht.h>

#include <vector>
#include <unordered_map>
#include <string>

#include "types.h"
#include "model_source.h"

class ZoneModelSource : public ModelSource
{
protected:
	std::vector<video::S3DVertex>* mNoCollisionVertexBuffers;
	std::vector<uint32>* mNoCollisionIndexBuffers;

protected:
	ZoneModelSource(S3D* s3d, std::string shortname);
	virtual ~ZoneModelSource();

	virtual void initMaterialBuffers() override;
};

#endif
