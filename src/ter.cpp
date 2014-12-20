
#include "ter.h"

TER::TER(MemoryStream* mem, S3D* s3d, std::string shortname) :
	ModelSource(s3d, shortname)
{
	byte* data = mem->getData();

	mHeader = (Header*)data;
	uint32 p = sizeof(Header);

	const char* m = mHeader->magic;
	if (m[0] != 'E' || m[1] != 'Q' || m[2] != 'G' || m[3] != 'T')
		throw ZEQException("TER::TER: File was not a valid TER");

	if (mHeader->version > 3)
		throw ZEQException("TER::TER: bad TER vesion (use TERv4)");

	mStringBlock = (char*)(data + p);
}

ZoneModel* TER::convertZoneModel()
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

	//create the irrlicht mesh, transferring buffers and creating final materials
	scene::SMesh* mesh = new scene::SMesh;
	scene::SMesh* nocollide_mesh = new scene::SMesh;
	ZoneModel* zone = new ZoneModel;

	for (uint32 i = 0; i < mNumMaterials; ++i)
	{
		if (!mMaterialVertexBuffers[i].empty())
			createMeshBuffer(mesh, mMaterialVertexBuffers[i], mMaterialIndexBuffers[i], &mMaterials[i], zone);
		if (!mNoCollisionVertexBuffers[i].empty())
			createMeshBuffer(nocollide_mesh, mNoCollisionVertexBuffers[i], mNoCollisionIndexBuffers[i], &mMaterials[i], zone);
	}

	mesh->recalculateBoundingBox();
	nocollide_mesh->recalculateBoundingBox();
	zone->setMeshes(mesh, nocollide_mesh);

	return zone;
}
