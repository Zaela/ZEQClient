
#include "model_source.h"

ModelSource::ModelSource(S3D* s3d, std::string shortname) :
	mContainingS3D(s3d),
	mStringBlock(nullptr),
	mShortName(shortname),
	mNumMaterials(0),
	mMaterials(nullptr),
	mMaterialVertexBuffers(nullptr),
	mMaterialIndexBuffers(nullptr)
{

}

ModelSource::~ModelSource()
{
	if (mMaterials)
		delete[] mMaterials;
	if (mMaterialVertexBuffers)
	{
		for (uint32 i = 0; i < mNumMaterials; ++i)
			mMaterialVertexBuffers[i].~vector();
		delete[] mMaterialVertexBuffers;
	}
	if (mMaterialIndexBuffers)
	{
		for (uint32 i = 0; i < mNumMaterials; ++i)
			mMaterialIndexBuffers[i].~vector();
		delete[] mMaterialIndexBuffers;
	}
	if (mNoCollisionVertexBuffers)
	{
		for (uint32 i = 0; i < mNumMaterials; ++i)
			mNoCollisionVertexBuffers[i].~vector();
		delete[] mNoCollisionVertexBuffers;
	}
	if (mNoCollisionIndexBuffers)
	{
		for (uint32 i = 0; i < mNumMaterials; ++i)
			mNoCollisionIndexBuffers[i].~vector();
		delete[] mNoCollisionIndexBuffers;
	}
}

void ModelSource::initMaterials(uint32 num)
{
	mNumMaterials = num;
	mMaterials = new IntermediateMaterial[num];
	for (uint32 i = 0; i < num; ++i)
		new (&mMaterials[i]) IntermediateMaterial;
}

void ModelSource::initMaterialBuffers()
{
	if (mMaterialVertexBuffers)
		return; //already created
	mMaterialVertexBuffers = new std::vector<video::S3DVertex>[mNumMaterials];
	mMaterialIndexBuffers = new std::vector<uint32>[mNumMaterials];
	mNoCollisionVertexBuffers = new std::vector<video::S3DVertex>[mNumMaterials];
	mNoCollisionIndexBuffers = new std::vector<uint32>[mNumMaterials];

	for (uint32 i = 0; i < mNumMaterials; ++i)
	{
		//placement new
		new (&mMaterialVertexBuffers[i]) std::vector<video::S3DVertex>;
		new (&mMaterialIndexBuffers[i]) std::vector<uint32>;
		new (&mNoCollisionVertexBuffers[i]) std::vector<video::S3DVertex>;
		new (&mNoCollisionIndexBuffers[i]) std::vector<uint32>;
	}
}

void ModelSource::createMeshBuffer(scene::SMesh* mesh, std::vector<video::S3DVertex>& vert_buf,
		std::vector<uint32>& index_buf, IntermediateMaterial* mat, Model* model)
{
	//irrlicht's default provided mesh buffers can take up to 65536 indices
	//need to handle case of total > 65535 by splitting into separate buffers
	uint32 count = index_buf.size();
	uint32 n = (count / 65535) + 1; //may want to make sure it's not an exact multiple
	uint32 adj = 0;

	AnimatedTexture* animTex = nullptr;
	if (mat->num_frames > 1 && model)
	{
		AnimatedTexture anim(mesh, mat, n, mesh->getMeshBufferCount());
		animTex = model->addAnimatedTexture(anim);
	}

	for (uint32 i = 0; i < n; ++i)
	{
		scene::SMeshBuffer* mesh_buffer = new scene::SMeshBuffer;
		auto& vbuf = mesh_buffer->Vertices;
		auto& ibuf = mesh_buffer->Indices;

		//65535 is a multiple of 3, use 0 to 65534
		uint32 max = count < 65535 ? count : 65535;

		for (uint32 j = 0; j < max; ++j)
		{
			vbuf.push_back(vert_buf[j + adj]);
			ibuf.push_back((uint16)(index_buf[j + adj] - adj));
		}

		//material
		video::SMaterial& material = mesh_buffer->getMaterial();
		if (mat)
		{
			if (mat->first.flag & IntermediateMaterialEntry::FULLY_TRANSPARENT)
			{
				material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF; //fully transparent materials should have no texture
				//use TRANSPARENT_VERTEX_ALPHA to show zone walls
			}
			else if (mat->first.diffuse_map)
			{
				material.setTexture(0, mat->first.diffuse_map);
				if (model) //temp
				model->addUsedTexture(mat->first.diffuse_map); //make a general Model class for this?

				if (mat->first.flag & IntermediateMaterialEntry::MASKED)
					material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF; //should be blended
				//put semi-transparent handling here (need to figure out how to do it and how to make it play nice with masking...)
				//probably make a copy of the texture and change the alpha of the bitmap pixels, then use blending EMT_TRANSPARENT_ALPHA_CHANNEL
			}
		}
		else
		{
			material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;
		}

		mesh_buffer->recalculateBoundingBox();
		mesh->addMeshBuffer(mesh_buffer);
		mesh_buffer->drop();

		adj += 65535;
		count -= 65535;
	}
}

uint32 ModelSource::readEQGMaterials(uint32 mat_count, byte* data, uint32 p)
{
	initMaterials(mat_count + 1); //include extra null material, at index = mat_count
	initMaterialBuffers();

	mMaterials[mat_count].first.flag = IntermediateMaterialEntry::FULLY_TRANSPARENT;

	auto& PropertyFunctions = getPropertyFunctions();

	//read materials
	for (uint32 m = 0; m < mat_count; ++m)
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

	return p;
}

void ModelSource::readEQGTriangles(uint32 mat_count, uint32 tri_count, Triangle* tris, Vertex* vertices, VertexV3* verticesV3)
{
	video::S3DVertex irrvert;
	std::vector<video::S3DVertex>* vert_buf;
	std::vector<uint32>* index_buf;

	for (uint32 i = 0; i < tri_count; ++i)
	{
		Triangle& tri = tris[i];
		if ((tri.flag & Triangle::PERMEABLE) == 0)
		{
			if (tri.material < 0)
			{
				//null material
				vert_buf = &mMaterialVertexBuffers[mat_count];
				index_buf = &mMaterialIndexBuffers[mat_count];
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
				vert_buf = &mNoCollisionVertexBuffers[mat_count];
				index_buf = &mNoCollisionIndexBuffers[mat_count];
			}
			else
			{
				vert_buf = &mNoCollisionVertexBuffers[tri.material];
				index_buf = &mNoCollisionIndexBuffers[tri.material];
			}
		}

		if (vertices)
		{
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
		}
		else
		{
			for (int j = 0; j < 3; ++j)
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
		}

		//winding order needs to be reversed
		uint32 s = index_buf->size() + 3;
		for (int j = 0; j < 3; ++j)
			index_buf->push_back(--s);
	}
}
