
#include "mob_model.h"

MobModel::MobModel() :
	mMesh(nullptr)
{

}

void MobModel::setMesh(int n, scene::ISkinnedMesh* mesh)
{
	if (n == 0)
	{
		//main mesh
		mMesh = mesh;
	}
	else
	{
		//head mesh
		mHeadMeshes.push_back(mesh);
	}
}
