
#include "wld_skeleton.h"
#include "mob.h"

WLDSkeleton::WLDSkeleton(uint32 num_bones, scene::SMesh* reference_mesh) :
	mIsCopy(false),
	mNumBones(num_bones),
	mNumFrames(0),
	mHasPoint(0),
	mReferenceMesh(reference_mesh)
{
	mBasePosition.bones = new BaseFrame[num_bones];

	mWeightsByBone = new std::vector<Weight>[num_bones];
	for (uint32 i = 0; i < num_bones; ++i)
		new (&mWeightsByBone[i]) std::vector<Weight>;
}

WLDSkeleton::WLDSkeleton(const WLDSkeleton& toCopy) :
	mIsCopy(true),
	mNumBones(toCopy.mNumBones),
	mNumFrames(toCopy.mNumFrames),
	mHasPoint(toCopy.mHasPoint)
{
	//copied WLDSkeletons share base positions and animation frames,
	//but not weights or reference meshes
	mBasePosition.bones = toCopy.mBasePosition.bones;

	for (uint32 i = 0; i < mNumFrames; ++i)
	{
		mFrames.push_back(toCopy.mFrames[i]);
	}

	mWeightsByBone = new std::vector<Weight>[mNumBones];
	for (uint32 i = 0; i < mNumBones; ++i)
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
		rot.X = -(float)f12.rotX / denom * 3.14159f * 0.5f;
		rot.Y = (float)f12.rotZ / denom * 3.14159f * 0.5f;
		rot.Z = -(float)f12.rotY / denom * 3.14159f * 0.5f;
	}
}

void WLDSkeleton::inheritPosRot(core::vector3df& pos, core::vector3df& rot, Frame& parent)
{
	//shifts are rotated according to parent's rotation
	Util::rotateBy(pos, parent.rot);
	//add parent's shifts
	pos += parent.pos;
	//add parent's rotations
	rot += parent.rot;
}

void WLDSkeleton::moveVertex(const video::S3DVertex& refVert, video::S3DVertex& vert, Frame& by)
{
	vert.Pos = refVert.Pos;
	vert.Normal = refVert.Normal;

	//rotate position
	Util::rotateBy(vert.Pos, by.rot);
	//add shifts
	vert.Pos += by.pos;

	//same for normals
	Util::rotateBy(vert.Normal, by.rot);
	vert.Normal += by.pos;
}

void WLDSkeleton::addWeight(int bone, uint32 buffer, uint32 vert)
{
	mWeightsByBone[bone].push_back(Weight(buffer, vert));
}

void WLDSkeleton::setBasePosition(int bone_id, Frag12Entry& f12, int parent_id, int point)
{
	core::vector3df pos, rot;
	getPosRot(f12, pos, rot);

	BaseFrame& bone = mBasePosition.bones[bone_id];
	if (parent_id != -1)
	{
		BaseFrame& parent = mBasePosition.bones[parent_id];
		inheritPosRot(pos, rot, parent.bone);
	}

	if (point != -1)
	{
		Frame* pointFrame = nullptr;
		switch (point)
		{
		case POINT_RIGHT:
			pointFrame = &mPoints[POINT_RIGHT_POS];
			//printf("found POINT_RIGHT\n");
			break;
		case POINT_LEFT:
			pointFrame = &mPoints[POINT_LEFT_POS];
			//printf("found POINT_LEFT\n");
			break;
		case POINT_SHIELD:
			pointFrame = &mPoints[POINT_SHIELD_POS];
			//printf("found POINT_SHIELD\n");
			break;
		case POINT_HEAD:
			pointFrame = &mPoints[POINT_HEAD_POS];
			//printf("found POINT_HEAD\n");
			break;
		}
		mHasPoint |= point;
		bone.attachment_point = pointFrame;
	}
	else
	{
		bone.attachment_point = nullptr;
	}

	bone.bone.pos = pos;
	bone.bone.rot = rot;
}

void WLDSkeleton::addAnimation(std::string animName, uint32 num_frames, uint32 frame_delay)
{
	Animation anim;
	anim.frame_delay = (float)frame_delay * 0.0005f; //hard to tell what the frame rate conversion factor is supposed to be
	anim.start_frame = mNumFrames;
	anim.end_frame = mNumFrames + num_frames - 1;

	mAnimations[animName] = anim;

	printf("created animation %s with %u frames from %u to %u with %g delay\n", animName.c_str(), num_frames,
		anim.start_frame, anim.end_frame, anim.frame_delay);

	mNumFrames += num_frames;
	for (uint32 i = 0; i < num_frames; ++i)
	{
		FrameSkeleton fs;
		fs.bones = new Frame[mNumBones];
		mFrames.push_back(fs);
	}
}

void WLDSkeleton::addAnimationFrames(std::string animName, Frag12* f12, int bone_id, int parent_id)
{
	core::vector3df pos, rot;
	Animation& anim = mAnimations[animName];
	uint32 num_frames = (anim.end_frame - anim.start_frame) + 1;

	if (f12)
	{
		uint32 count;
		Frame* frame;
		for (count = 0; count < f12->count; ++count)
		{
			uint32 n = anim.start_frame + count;
			frame = &mFrames[n].bones[bone_id];
			Frag12Entry& ent = f12->entry[count];
			getPosRot(ent, pos, rot);

			if (parent_id != -1)
			{
				Frame& parent = mFrames[n].bones[parent_id];
				inheritPosRot(pos, rot, parent);
			}

			frame->pos = pos;
			frame->rot = rot;
		}

		while (count < num_frames)
		{
			//less frames than expected (i.e., 1)
			//fill remaining frames with a copy of the last frame
			Frame& fr = mFrames[anim.start_frame + count].bones[bone_id];
			fr.pos = frame->pos;
			fr.rot = frame->rot;

			++count;
		}
	}
	else
	{
		//attachment point, no frag12
		//all frames are our base position modified by the frame of our parent
		Frame& point = mBasePosition.bones[bone_id].bone;

		for (uint32 count = 0; count < num_frames; ++count)
		{
			pos = point.pos;
			rot = point.rot;

			uint32 n = anim.start_frame + count;
			Frame& parent = mFrames[n].bones[parent_id];
			inheritPosRot(pos, rot, parent);

			Frame& frame = mFrames[n].bones[bone_id];
			frame.pos = pos;
			frame.rot = rot;
		}
	}
}

void WLDSkeleton::assumeBasePosition(scene::SMesh* mesh)
{
	//for each bone...
	for (uint32 b = 0; b < mNumBones; ++b)
	{
		BaseFrame& base = mBasePosition.bones[b];
		Frame& bone = base.bone;
		//for each vertex assigned to that bone
		std::vector<Weight>& weights = mWeightsByBone[b];
		uint16 prev_index = 65535;
		video::S3DVertex* refVerts;
		video::S3DVertex* verts;
		for (Weight& wt : weights)
		{
			if (wt.buffer_index != prev_index)
			{
				refVerts = (video::S3DVertex*)mReferenceMesh->getMeshBuffer(wt.buffer_index)->getVertices();
				verts = (video::S3DVertex*)mesh->getMeshBuffer(wt.buffer_index)->getVertices();
				prev_index = wt.buffer_index;
			}

			const video::S3DVertex& refVert = refVerts[wt.vert_index];
			video::S3DVertex& vert = verts[wt.vert_index];
			
			moveVertex(refVert, vert, bone);
		}

		if (base.attachment_point)
		{
			base.attachment_point->pos = bone.pos;
			base.attachment_point->rot = bone.rot;
		}
	}
}

uint32 WLDSkeleton::animate(std::string animName, scene::SMesh* mesh, uint32 cur_frame, float& time, bool& ended)
{
	Animation& anim = mAnimations[animName];
	uint32 num_frames = (anim.end_frame - anim.start_frame) + 1;

	if (time >= anim.frame_delay)
	{
		//advance frame
		uint32 advanceBy = 0;
		while (time > anim.frame_delay)
		{
			time -= anim.frame_delay;
			++advanceBy;
		}

		cur_frame += advanceBy;
		while (cur_frame >= num_frames)
			cur_frame -= num_frames;
	}

	float percent;
	uint32 next_frame = cur_frame + 1;
	if (next_frame == num_frames)
	{
		percent = 1.0f;
		ended = true;
	}
	else
	{
		percent = time / anim.frame_delay;
	}

	FrameSkeleton& curFrame = mFrames[anim.start_frame + cur_frame];

	if (percent < 1.0f)
	{
		//interpolate
		FrameSkeleton& nextFrame = mFrames[anim.start_frame + next_frame];

		//for each bone...
		for (uint32 b = 0; b < mNumBones; ++b)
		{
			Frame& cur = curFrame.bones[b];
			Frame& next = nextFrame.bones[b];

			Frame moveFrame;
			moveFrame.pos = cur.pos + (next.pos - cur.pos) * percent;
			moveFrame.rot = cur.rot + (next.rot - cur.rot) * percent;

			BaseFrame& base = mBasePosition.bones[b];
			//if it's not an attachment point, move vertices
			if (base.attachment_point == nullptr)
			{
				//for each vertex assigned to that bone
				std::vector<Weight>& weights = mWeightsByBone[b];
				uint16 prev_index = 65535;
				video::S3DVertex* refVerts;
				video::S3DVertex* verts;
				for (Weight& wt : weights)
				{
					if (wt.buffer_index != prev_index)
					{
						refVerts = (video::S3DVertex*)mReferenceMesh->getMeshBuffer(wt.buffer_index)->getVertices();
						verts = (video::S3DVertex*)mesh->getMeshBuffer(wt.buffer_index)->getVertices();
						prev_index = wt.buffer_index;
					}

					const video::S3DVertex& refVert = refVerts[wt.vert_index];
					video::S3DVertex& vert = verts[wt.vert_index];
			
					moveVertex(refVert, vert, moveFrame);
				}
			}
			else
			{
				//it's an attachment point, just write the interpolated values (maybe?)
				base.attachment_point->pos = moveFrame.pos;
				base.attachment_point->rot = moveFrame.rot;
			}
		}
	}
	else
	{
		//no interpolation needed
		//for each bone...
		for (uint32 b = 0; b < mNumBones; ++b)
		{
			Frame& cur = curFrame.bones[b];

			BaseFrame& base = mBasePosition.bones[b];
			//if it's not an attachment point, move vertices
			if (base.attachment_point == nullptr)
			{
				//for each vertex assigned to that bone
				std::vector<Weight>& weights = mWeightsByBone[b];
				uint16 prev_index = 65535;
				video::S3DVertex* refVerts;
				video::S3DVertex* verts;
				for (Weight& wt : weights)
				{
					if (wt.buffer_index != prev_index)
					{
						refVerts = (video::S3DVertex*)mReferenceMesh->getMeshBuffer(wt.buffer_index)->getVertices();
						verts = (video::S3DVertex*)mesh->getMeshBuffer(wt.buffer_index)->getVertices();
						prev_index = wt.buffer_index;
					}

					const video::S3DVertex& refVert = refVerts[wt.vert_index];
					video::S3DVertex& vert = verts[wt.vert_index];
			
					moveVertex(refVert, vert, cur);
				}
			}
			else
			{
				//it's an attachment point, just write the interpolated values (maybe?)
				base.attachment_point->pos = cur.pos;
				base.attachment_point->rot = cur.rot;
			}
		}
	}

	return cur_frame;
}


WLDSkeletonInstance::WLDSkeletonInstance(scene::SMesh* mesh, WLDSkeleton* skele) :
	mMesh(mesh),
	mCurTime(0),
	mCurFrame(0),
	mSkeleton(skele)
{

}

WLDSkeletonInstance::~WLDSkeletonInstance()
{
	mMesh->drop();
}

void WLDSkeletonInstance::assumeBasePosition()
{
	mSkeleton->assumeBasePosition(mMesh);
}

void WLDSkeletonInstance::setAnimation(std::string animName)
{
	mCurAnimation = animName;
	mCurFrame = 0;
	mCurTime = 0.0f;
}

void WLDSkeletonInstance::animate(float delta)
{
	if (mCurAnimation.empty())
		return;
	bool ended = false;
	mCurTime += delta;
	mCurFrame = mSkeleton->animate(mCurAnimation, mMesh, mCurFrame, mCurTime, ended);

	if (ended)
		setAnimation(mCurAnimation);
}
