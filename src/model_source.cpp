
#include "model_source.h"

ModelSource::ModelSource() :
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

	for (uint32 i = 0; i < mNumMaterials; ++i)
	{
		//placement new
		new (&mMaterialVertexBuffers[i]) std::vector<video::S3DVertex>;
		new (&mMaterialIndexBuffers[i]) std::vector<uint32>;
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
