
#include "mod.h"

MOD::MOD(MemoryStream* mem, S3D* s3d, std::string shortname) :
	ModelSource(s3d, shortname)
{
	byte* data = mem->getData();

	mHeader = (Header*)data;
	uint32 p = sizeof(Header);

	const char* m = mHeader->magic;
	if (m[0] != 'E' || m[1] != 'Q' || m[2] != 'G' || m[3] != 'M')
		throw ZEQException("MOD::MOD: File was not a valid MOD");

	mStringBlock = (char*)(data + p);
}

void MOD::convertStaticModel(ZoneModel* zoneModel)
{
	byte* data = (byte*)mHeader;
	uint32 p = sizeof(Header) + mHeader->strings_len;

	//materials
	p = readEQGMaterials(mHeader->material_count, data, p);

	//vertices
	Vertex* vertices = nullptr;
	VertexV3* verticesV3 = nullptr;
	if (mHeader->version < 3)
	{
		vertices = (Vertex*)(data + p);
		p += sizeof(Vertex) * mHeader->vertex_count;
	}
	else
	{
		verticesV3 = (VertexV3*)(data + p);
		p += sizeof(VertexV3) * mHeader->vertex_count;
	}

	//triangles
	Triangle* tris = (Triangle*)(data + p);
	readEQGTriangles(mHeader->material_count, mHeader->triangle_count, tris, vertices, verticesV3);

	//create irrlicht meshes
	scene::SMesh* mesh = new scene::SMesh;
	scene::SMesh* noncollision_mesh = new scene::SMesh;
	//find any vertex + index buffers with data in them and use them to fill this mesh
	for (uint32 i = 0; i < mNumMaterials; ++i)
	{
		if (!mMaterialVertexBuffers[i].empty())
		{
			createMeshBuffer(mesh, mMaterialVertexBuffers[i], mMaterialIndexBuffers[i], &mMaterials[i], zoneModel);
			mMaterialVertexBuffers[i].clear();
			mMaterialIndexBuffers[i].clear();
		}
		if (!mNoCollisionVertexBuffers[i].empty())
		{
			createMeshBuffer(noncollision_mesh, mNoCollisionVertexBuffers[i], mNoCollisionIndexBuffers[i], &mMaterials[i], zoneModel);
			mNoCollisionVertexBuffers[i].clear();
			mNoCollisionIndexBuffers[i].clear();
		}
	}

	if (mesh->getMeshBufferCount() > 0)
	{
		mesh->recalculateBoundingBox();
		zoneModel->addObjectDefinition(mShortName.c_str(), mesh);
	}

	if (noncollision_mesh->getMeshBufferCount() > 0)
	{
		noncollision_mesh->recalculateBoundingBox();
		zoneModel->addNoCollisionObjectDefinition(mShortName.c_str(), noncollision_mesh);
	}
}
