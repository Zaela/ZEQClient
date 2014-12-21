
#include "wld_skeleton.h"

WLDSkeleton::WLDSkeleton(uint32 num_bones) :
	mNumBones(num_bones)
{
	mBasePosition.bones = new Frame[num_bones];

	mWeightsByBone = new std::vector<Weight>[num_bones];
	for (uint32 i = 0; i < num_bones; ++i)
		new (&mWeightsByBone[i]) std::vector<Weight>;
}

void WLDSkeleton::getPosRot(Frag12Entry& f12, core::vector3df& pos, core::vector3df& rot)
{
	float denom;
	if (f12.shiftDenom != 0)
	{
		denom = (float)f12.shiftDenom;
		pos.X = (float)f12.shiftX / denom;
		pos.Y = (float)f12.shiftZ / denom;
		pos.Z = (float)f12.shiftY / denom;
	}

	if (f12.rotDenom != 0)
	{
		denom = (float)f12.rotDenom;
		rot.X = (float)f12.rotX / denom * 3.14159f * 0.5f;
		rot.Y = (float)f12.rotZ / denom * 3.14159f * 0.5f;
		rot.Z = (float)f12.rotY / denom * 3.14159f * 0.5f;
	}
}

void WLDSkeleton::addWeight(int bone, uint32 buffer, uint32 vert)
{
	mWeightsByBone[bone].push_back(Weight(buffer, vert));
}

void WLDSkeleton::setBasePosition(int bone_id, Frag12Entry& f12, int parent_id)
{
	core::vector3df pos, rot;
	getPosRot(f12, pos, rot);

	Frame& bone = mBasePosition.bones[bone_id];
	if (parent_id != -1)
	{
		Frame& parent = mBasePosition.bones[parent_id];
		//shifts are rotated according to parent's rotation
		Util::rotateBy(pos, parent.rot);
		//add parent's shifts
		pos += parent.pos;
		//add parent's rotations
		rot += parent.rot;
	}

	bone.pos = pos;
	bone.rot = rot;
}

void WLDSkeleton::assumeBasePosition(scene::SMesh* mesh)
{
	//for each bone...
	for (uint32 b = 0; b < mNumBones; ++b)
	{
		Frame& bone = mBasePosition.bones[b];
		//for each vertex assigned to that bone
		std::vector<Weight>& weights = mWeightsByBone[b];
		uint16 prev_index = 65535;
		scene::IMeshBuffer* buf;
		for (Weight& wt : weights)
		{
			if (wt.buffer_index != prev_index)
			{
				buf = mesh->getMeshBuffer(wt.buffer_index);
				prev_index = wt.buffer_index;
			}

			video::S3DVertex& vert = ((video::S3DVertex*)buf->getVertices())[wt.vert_index];
			//rotate position
			Util::rotateBy(vert.Pos, bone.rot);
			//add shifts
			vert.Pos += bone.pos;

			//same for normals
			Util::rotateBy(vert.Normal, bone.rot);
			vert.Normal += bone.pos;
		}
	}
}


WLDSkeletonInstance::WLDSkeletonInstance(scene::SMesh* mesh, WLDSkeleton* skele) :
	mMesh(mesh),
	mCurTime(0),
	mCurFrame(0),
	mSkeleton(skele)
{

}

void WLDSkeletonInstance::assumeBasePosition()
{
	mSkeleton->assumeBasePosition(mMesh);
}
