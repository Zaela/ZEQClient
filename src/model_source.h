
#ifndef _ZEQ_MODEL_SOURCE_H
#define _ZEQ_MODEL_SOURCE_H

#include <irrlicht.h>

#include <vector>
#include <string>

#include "types.h"
#include "structs_intermediate.h"
#include "model.h"
#include "s3d.h"

using namespace irr;

class ModelSource
{
protected:
	S3D* mContainingS3D;
	char* mStringBlock;
	std::string mShortName;

	uint32 mNumMaterials;
	IntermediateMaterial* mMaterials;

	std::vector<video::S3DVertex>* mMaterialVertexBuffers;
	std::vector<uint32>* mMaterialIndexBuffers;

protected:
	ModelSource(S3D* s3d, std::string shortname);
	virtual ~ModelSource();

	void initMaterials(uint32 num);
	virtual void initMaterialBuffers();
	static void createMeshBuffer(scene::SMesh* mesh, std::vector<video::S3DVertex>& vert_buf,
		std::vector<uint32>& index_buf, IntermediateMaterial* mat = nullptr, Model* zone = nullptr);

public:
	S3D* getContainingS3D() { return mContainingS3D; }
	char* getStringBlock() { return mStringBlock; }
	std::string& getShortName() { return mShortName; }
};

#endif
