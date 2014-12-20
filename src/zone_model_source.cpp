
#include "zone_model_source.h"

ZoneModelSource::ZoneModelSource() :
	mNoCollisionVertexBuffers(nullptr),
	mNoCollisionIndexBuffers(nullptr)
{

}

ZoneModelSource::~ZoneModelSource()
{
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

void ZoneModelSource::initMaterialBuffers()
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
