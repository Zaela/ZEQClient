
#ifndef _ZEQ_MODEL_SOURCE_H
#define _ZEQ_MODEL_SOURCE_H

#include <irrlicht.h>

#include <vector>

#include "types.h"
#include "structs_intermediate.h"
#include "model.h"

using namespace irr;

class ModelSource
{
protected:
	uint32 mNumMaterials;
	IntermediateMaterial* mMaterials;

	std::vector<video::S3DVertex>* mMaterialVertexBuffers;
	std::vector<uint32>* mMaterialIndexBuffers;

protected:
	ModelSource();
	virtual ~ModelSource();

	void initMaterials(uint32 num);
	virtual void initMaterialBuffers();
	static void createMeshBuffer(scene::SMesh* mesh, std::vector<video::S3DVertex>& vert_buf,
		std::vector<uint32>& index_buf, IntermediateMaterial* mat = nullptr, Model* zone = nullptr);
};

#endif
