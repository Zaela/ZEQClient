
#include "mob_model.h"

MobModel::MobModel() :
	mMesh(nullptr)
{

}

void MobModel::setMesh(int n, scene::SMesh* mesh)
{
	scene::SAnimatedMesh* animMesh = new scene::SAnimatedMesh(mesh);
	mesh->drop();

	if (n == 0)
	{
		//main mesh
		mMesh = animMesh;
	}
	else
	{
		//head mesh
		mHeadMeshes.push_back(animMesh);
	}
}
