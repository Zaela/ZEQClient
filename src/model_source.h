
#ifndef _ZEQ_MODEL_SOURCE_H
#define _ZEQ_MODEL_SOURCE_H

#include <irrlicht.h>

#include <vector>
#include <string>

#include "types.h"
#include "structs_intermediate.h"
#include "model.h"
#include "s3d.h"
#include "structs_eqg.h"

using namespace irr;
using namespace EQG_Structs;

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
	std::vector<video::S3DVertex>* mNoCollisionVertexBuffers;
	std::vector<uint32>* mNoCollisionIndexBuffers;

protected:
	ModelSource(S3D* s3d, std::string shortname);
	virtual ~ModelSource();

	void initMaterials(uint32 num);
	void initMaterialBuffers();
	static void createMeshBuffer(scene::SMesh* mesh, std::vector<video::S3DVertex>& vert_buf,
		std::vector<uint32>& index_buf, IntermediateMaterial* mat = nullptr, Model* zone = nullptr);

	uint32 readEQGMaterials(uint32 mat_count, byte* data, uint32 p);
	void readEQGTriangles(uint32 mat_count, uint32 tri_count, Triangle* tris, Vertex* vertices, VertexV3* verticesV3);


public:
	S3D* getContainingS3D() { return mContainingS3D; }
	char* getStringBlock() { return mStringBlock; }
	std::string& getShortName() { return mShortName; }
};

#endif
