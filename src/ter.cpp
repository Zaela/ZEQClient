
#include "ter.h"

TER::TER(MemoryStream* mem, S3D* s3d, std::string shortname) :
	mContainingS3D(s3d),
	mShortName(shortname)
{
	byte* data = mem->getData();

	mHeader = (Header*)data;
	uint32 p = sizeof(Header);

	const char* m = mHeader->magic;
	if (m[0] != 'E' || m[1] != 'Q' || m[2] != 'G' || m[3] != 'T')
		throw ZEQException("TER::TER: File was not a valid TER");

	if (mHeader->version > 3)
		throw ZEQException("TER::TER: bad TER vesion (use TERv4)");

	mStringBlock = (const char*)(data + p);
}

ZoneModel* TER::convertZoneModel()
{
	byte* data = (byte*)mHeader;
	uint32 p = sizeof(Header) + mHeader->strings_len;

	initMaterials(mHeader->material_count + 1); //include extra null material, at index = mHeader->material_count
	initMaterialBuffers();

	mMaterials[mHeader->material_count].first.flag = IntermediateMaterialEntry::FULLY_TRANSPARENT;

	auto& PropertyFunctions = getPropertyFunctions();

	//read materials
	for (uint32 m = 0; m < mHeader->material_count; ++m)
	{
		Material* mat = (Material*)(data + p);
		p += sizeof(Material);

		for (uint32 i = 0; i < mat->property_count; ++i)
		{
			Property* prop = (Property*)(data + p);
			p += sizeof(Property);
			const char* prop_name = &mStringBlock[prop->name_index];

			if (PropertyFunctions.count(prop_name) != 0)
				PropertyFunctions[prop_name](prop, mMaterials[m].first, this);
		}
	}

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
	video::S3DVertex irrvert;
	std::vector<video::S3DVertex>* vert_buf;
	std::vector<uint32>* index_buf;
	Triangle* tris = (Triangle*)(data + p);

	//not the most space-efficient code, but helps keep it fast
	if (vertices)
	{
		for (uint32 i = 0; i < mHeader->triangle_count; ++i)
		{
			Triangle& tri = tris[i];
			if ((tri.flag & Triangle::PERMEABLE) == 0)
			{
				if (tri.material < 0)
				{
					//null material
					vert_buf = &mMaterialVertexBuffers[mHeader->material_count];
					index_buf = &mMaterialIndexBuffers[mHeader->material_count];
				}
				else
				{
					vert_buf = &mMaterialVertexBuffers[tri.material];
					index_buf = &mMaterialIndexBuffers[tri.material];
				}
			}
			else
			{
				if (tri.material < 0)
				{
					//null material
					vert_buf = &mNoCollisionVertexBuffers[mHeader->material_count];
					index_buf = &mNoCollisionIndexBuffers[mHeader->material_count];
				}
				else
				{
					vert_buf = &mNoCollisionVertexBuffers[tri.material];
					index_buf = &mNoCollisionIndexBuffers[tri.material];
				}
			}

			for (int j = 0; j < 3; ++j)
			{
				Vertex& vert = vertices[tri.index[j]];
				irrvert.Pos.X = vert.x;
				irrvert.Pos.Y = vert.z;
				irrvert.Pos.Z = vert.y;
				irrvert.Normal.X = vert.i;
				irrvert.Normal.Y = vert.k;
				irrvert.Normal.Z = vert.j;
				irrvert.TCoords.X = vert.u;
				irrvert.TCoords.Y = vert.v;
				vert_buf->push_back(irrvert);
			}

			for (int j = 0; j < 3; ++j)
				index_buf->push_back(index_buf->size());
		}
	}
	else
	{
		for (uint32 i = 0; i < mHeader->triangle_count; ++i)
		{
			Triangle& tri = tris[i];
			if ((tri.flag & Triangle::PERMEABLE) == 0)
			{
				if (tri.material < 0)
				{
					//null material
					vert_buf = &mMaterialVertexBuffers[mHeader->material_count];
					index_buf = &mMaterialIndexBuffers[mHeader->material_count];
				}
				else
				{
					vert_buf = &mMaterialVertexBuffers[tri.material];
					index_buf = &mMaterialIndexBuffers[tri.material];
				}
			}
			else
			{
				if (tri.material < 0)
				{
					//null material
					vert_buf = &mNoCollisionVertexBuffers[mHeader->material_count];
					index_buf = &mNoCollisionIndexBuffers[mHeader->material_count];
				}
				else
				{
					vert_buf = &mNoCollisionVertexBuffers[tri.material];
					index_buf = &mNoCollisionIndexBuffers[tri.material];
				}
			}

			for (int j = 0; j < 2; ++j)
			{
				VertexV3& vert = verticesV3[tri.index[j]];
				irrvert.Pos.X = vert.x;
				irrvert.Pos.Y = vert.z;
				irrvert.Pos.Z = vert.y;
				irrvert.Normal.X = vert.i;
				irrvert.Normal.Y = vert.k;
				irrvert.Normal.Z = vert.j;
				irrvert.TCoords.X = vert.u;
				irrvert.TCoords.Y = vert.v;
				vert_buf->push_back(irrvert);
			}

			for (int j = 0; j < 2; ++j)
				index_buf->push_back(index_buf->size());
		}
	}

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
