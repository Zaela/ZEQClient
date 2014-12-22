
#include "wld.h"

extern Renderer gRenderer;
extern MobManager gMobMgr;

WLD::WLD(MemoryStream* mem, S3D* s3d, std::string shortname) :
	ModelSource(s3d, shortname)
{
	byte* data = mem->getData();

	mHeader = (Header*)data;
	uint32 p = sizeof(Header);

	const char* m = mHeader->magic;
	if (m[0] != 0x02 || m[1] != 0x3D || m[2] != 0x50 || m[3] != 0x54)
		throw ZEQException("WLD::WLD: File was not a valid WLD");

	mHeader->version &= 0xFFFFFFFE;
	if (mHeader->version != Header::VERSION1 && mHeader->version != Header::VERSION2)
		throw ZEQException("WLD::WLD: File was not a valid WLD version");
	mVersion = (mHeader->version == Header::VERSION1) ? 1 : 2;

	char* strings = (char*)&data[p];
	decodeString(strings, mHeader->strings_len);
	mStringBlock = strings;
	p += mHeader->strings_len;

	//zeroth fragment is null - saves us from doing a lot of 'ref - 1' later on
	mFragsByIndex.push_back(nullptr);

	const uint32 count = mHeader->frag_count;
	for (uint32 i = 0; i < count; ++i)
	{
		FragHeader* fh = (FragHeader*)&data[p];

		mFragsByIndex.push_back(fh);
		mFragsByType[fh->type].push_back(fh);
		if (fh->nameref < 0)
			mFragsByNameRef[-fh->nameref] = fh;

		p += FragHeader::SIZE + fh->len;
	}
}

void WLD::decodeString(void* str, size_t len)
{
	static byte hashval[] = {0x95, 0x3A, 0xC5, 0x2A, 0x95, 0x7A, 0x95, 0x6A};
	byte* data = (byte*)str;
	for (size_t i = 0; i < len; ++i)
	{
		data[i] ^= hashval[i & 7];
	}
}

FragHeader* WLD::getFragByRef(int ref)
{
	if (ref > 0)
	{
		return mFragsByIndex[ref];
	}
	else
	{
		if (ref == 0)
			ref = -1;
		if (mFragsByNameRef.count(ref))
			return mFragsByNameRef[ref];
	}

	return nullptr;
}

const char* WLD::getFragName(FragHeader* frag)
{
	return getFragName(frag->nameref);
}

const char* WLD::getFragName(int ref)
{
	uint32 r = -ref;
	if (ref >= 0 || r >= mHeader->strings_len)
		return nullptr;
	return mStringBlock - ref;
}

void WLD::processMaterials()
{
	if (mMaterials || mFragsByType.count(0x03) == 0)
		return;

	//pre-process 0x03 frags so their strings are only decoded once each
	for (FragHeader* frag : mFragsByType[0x03])
	{
		Frag03* f03 = (Frag03*)frag;
		if (f03->string_len == 0)
			continue;

		decodeString(f03->string, f03->string_len);
		Util::toLower((char*)f03->string, f03->string_len);
		mTexturesByFrag03[f03] = (const char*)f03->string;
	}

	//process 0x30 frags to find all materials in the wld
	if (mFragsByType.count(0x30) == 0)
		return;

	initMaterials(mFragsByType[0x30].size());

	int i = -1;
	for (FragHeader* frag : mFragsByType[0x30])
	{
		Frag30* f30 = (Frag30*)frag;
		Frag03* f03;

		IntermediateMaterial* mat = &mMaterials[++i];

		mMaterialIndicesByFrag30[f30] = i;
		
		//f30 -> f05 -> f04 -> f03
		//OR
		//f30 -> f03 (may have null texture)
		if (f30->ref > 0)
		{
			Frag05* f05 = (Frag05*)getFragByRef(f30->ref);
			if (f05 == nullptr)
				continue;
			Frag04* f04 = (Frag04*)getFragByRef(f05->ref);
			if (f04 == nullptr)
				continue;
			//we have our f04, check if it's animated
			if (!f04->isAnimated())
			{
				f03 = (Frag03*)getFragByRef(f04->ref);
			}
			else
			{
				handleAnimatedMaterial(f04, f30, mat);
				continue;
			}
		}
		else
		{
			f03 = (Frag03*)getFragByRef(f30->ref);
		}

		//we have our f03 frag, time to make our material
		Frag03ToMaterialEntry(f03, f30, &mat->first);
	}
}

void WLD::Frag03ToMaterialEntry(Frag03* f03, Frag30* f30, IntermediateMaterialEntry* mat_ent)
{
	if (f03 && f03->string_len > 0)
	{
		//we have a texture name, create it in the renderer
		MemoryStream* file = mContainingS3D->getFile((char*)f03->string);
		if (file == nullptr)
		{
			printf("Could not find texture '%s'\n", (char*)f03->string);
			return;
		}
		std::string name = mShortName;
		name += '/';
		name += getFragName((FragHeader*)f30);
		bool isDDS = false;

		mat_ent->diffuse_map = gRenderer.createTexture(file, name, isDDS);

		mat_ent->flag = translateVisibilityFlag(f30, isDDS);
	}
	else
	{
		//null material
		mat_ent->flag = IntermediateMaterialEntry::FULLY_TRANSPARENT;
	}
}

void WLD::handleAnimatedMaterial(Frag04* f04, Frag30* f30, IntermediateMaterial* mat)
{
	Frag04Animated* f04a = (Frag04Animated*)f04;

	mat->num_frames = f04a->count;
	mat->frame_delay = f04a->milliseconds;
	int add = f04a->count - 1;
	mat->additional = new IntermediateMaterialEntry[add];
	for (int i = 0; i < add; ++i)
		new (&mat->additional[i]) IntermediateMaterialEntry;

	int* ref_ptr = f04a->getRefList();

	//first is outside the array
	IntermediateMaterialEntry* mat_ent = &mat->first;
	Frag03* f03 = (Frag03*)getFragByRef(*ref_ptr++);
	Frag03ToMaterialEntry(f03, f30, mat_ent);

	for (int i = 0; i < add; ++i)
	{
		mat_ent = &mat->additional[i];
		f03 = (Frag03*)getFragByRef(*ref_ptr++);
		Frag03ToMaterialEntry(f03, f30, mat_ent);
	}
}

uint32 WLD::translateVisibilityFlag(Frag30* f30, bool isDDS)
{
	uint32 ret = 0;

	if (f30->visibility_flag == 0)
		return IntermediateMaterialEntry::FULLY_TRANSPARENT;
	if ((f30->visibility_flag & Frag30::MASKED) == Frag30::MASKED || (f30->visibility_flag & 0xB) == 0xB)
		ret |= IntermediateMaterialEntry::MASKED;
	if ((f30->visibility_flag & Frag30::SEMI_TRANSPARENT) == Frag30::SEMI_TRANSPARENT)
		ret |= IntermediateMaterialEntry::SEMI_TRANSPARENT;
	if (isDDS)
		ret |= IntermediateMaterialEntry::DDS_TEXTURE;

	return ret;
}

void WLD::processMesh(Frag36* f36, WLDSkeleton* skeleton)
{
	byte* data = (byte*)f36;
	uint32 p = sizeof(Frag36);

	const float scale = 1.0f / (1 << f36->scale);

	//for bone assignments
	std::vector<uint16> vertToBoneAssignment;

	//raw vertices
	RawVertex* wld_verts = (RawVertex*)(data + p);
	p += sizeof(RawVertex) * f36->vert_count;

	//raw uvs
	RawUV16* uv16 = nullptr;
	RawUV32* uv32 = nullptr;
	if (f36->uv_count > 0)
	{
		if (getVersion() == 1)
		{
			uv16 = (RawUV16*)(data + p);
			p += sizeof(RawUV16) * f36->uv_count;
		}
		else
		{
			uv32 = (RawUV32*)(data + p);
			p += sizeof(RawUV32) * f36->uv_count;
		}
	}

	//raw normals
	RawNormal* wld_norm = (RawNormal*)(data + p);
	p += sizeof(RawNormal) * f36->vert_count;

	//skip vertex colors (for now?)
	p += sizeof(uint32) * f36->color_count;

	//raw triangles
	RawTriangle* wld_tris = (RawTriangle*)(data + p);
	p += sizeof(RawTriangle) * f36->poly_count;

	//bone assignments
	if (skeleton)
	{
		WLD_Structs::BoneAssignment* ba = (WLD_Structs::BoneAssignment*)(data + p);
		for (uint16 b = 0; b < f36->bone_assignment_count; ++b)
		{
			for (uint16 i = 0; i < ba->count; ++i)
				vertToBoneAssignment.push_back(ba->index);
			++ba;
		}
	}
	p += sizeof(WLD_Structs::BoneAssignment) * f36->bone_assignment_count;

	//get material indices for triangles
	std::unordered_map<uint32, int> mat_index_list;
	Frag31* f31 = (Frag31*)getFragByRef(f36->texture_list_ref);
	int* ref_ptr = f31->getRefList();
	for (uint32 i = 0; i < f31->ref_count; ++i)
	{
		Frag30* f30 = (Frag30*)getFragByRef(*ref_ptr++);
		mat_index_list[i] = mMaterialIndicesByFrag30[f30];
	}

	auto processTriangle = [=, &vertToBoneAssignment](RawTriangle& tri, std::vector<video::S3DVertex>& vert_buf,
		std::vector<uint32>& index_buf, int mat_index)
	{
		uint32 base = vert_buf.size();

		//handle uv conversions
		video::S3DVertex vertex;
		if (uv16)
		{
			for (int i = 0; i < 3; ++i)
			{
				static const float uv_scale = 1.0f / 256.0f;
				RawUV16& uv = uv16[tri.index[i]];
				vertex.TCoords.X = (float)uv.u * uv_scale;
				vertex.TCoords.Y = -((float)uv.v * uv_scale);
				vert_buf.push_back(vertex);
			}
		}
		else if (uv32)
		{
			for (int i = 0; i < 3; ++i)
			{
				RawUV32& uv = uv32[tri.index[i]];
				vertex.TCoords.X = uv.u;
				vertex.TCoords.Y = -uv.v;
				vert_buf.push_back(vertex);
			}
		}
		else
		{
			vertex.TCoords.X = 0;
			vertex.TCoords.Y = 0;
			for (int i = 0; i < 3; ++i)
				vert_buf.push_back(vertex);
		}

		//handle vertex and normal conversions
		uint32 buf_pos = base;
		for (int i = 0; i < 3; ++i)
		{
			video::S3DVertex& vertex = vert_buf[buf_pos];
			uint16 idx = tri.index[i];
			RawVertex& vert = wld_verts[idx];
			vertex.Pos.X = f36->x + (float)vert.x * scale;
			vertex.Pos.Z = f36->y + (float)vert.y * scale; //irrlicht uses Y for the "up" axis, need to switch
			vertex.Pos.Y = f36->z + (float)vert.z * scale;
			RawNormal& norm = wld_norm[idx];
			static const float normal_scale = 1.0f / 127.0f;
			vertex.Normal.X = (float)norm.i * normal_scale;
			vertex.Normal.Z = (float)norm.j * normal_scale;
			vertex.Normal.Y = (float)norm.k * normal_scale;

			if (skeleton)
			{
				skeleton->addWeight(vertToBoneAssignment[idx], mat_index, buf_pos);
			}

			++buf_pos;
		}

		//write indices (purely in order for now, no reused vertices)
		//would be faster to do them all per material in one step, in that case... but we should look at reusing vertices instead
		for (int i = 0; i < 3; ++i)
			index_buf.push_back(base++);
	};

	//construct vertices and triangles based on their materials
	for (uint16 m = 0; m < f36->poly_texture_count; ++m)
	{
		RawTextureEntry* rte = (RawTextureEntry*)(data + p);
		p += sizeof(RawTextureEntry);

		int mat_index = mat_index_list[rte->index];

		//get buffers
		std::vector<video::S3DVertex>& vert_buf = mMaterialVertexBuffers[mat_index];
		std::vector<uint32>& index_buf = mMaterialIndexBuffers[mat_index];
		std::vector<video::S3DVertex>& nocollide_vert_buf = mNoCollisionVertexBuffers[mat_index];
		std::vector<uint32>& nocollide_index_buf = mNoCollisionIndexBuffers[mat_index];

		for (uint16 i = 0; i < rte->count; ++i)
		{
			RawTriangle& tri = wld_tris[i];
			//if ((tri.flag & RawTriangle::PERMEABLE) == 0 || skele)
			if ((tri.flag & RawTriangle::PERMEABLE) == 0 || skeleton)
				processTriangle(tri, vert_buf, index_buf, mat_index);
			else
				processTriangle(tri, nocollide_vert_buf, nocollide_index_buf, mat_index);
		}

		//advance triangles ptr for the next block
		wld_tris += rte->count;
	}
}

ZoneModel* WLD::convertZoneModel()
{
	if (mFragsByType.count(0x36) == 0)
		return nullptr;

	processMaterials();
	
	//initialize two buffers for each material
	initMaterialBuffers();
	//should also make a collision mesh buffer here...
	
	//process mesh fragments
	for (FragHeader* frag : mFragsByType[0x36])
	{
		processMesh((Frag36*)frag);
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

void WLD::convertZoneObjectDefinitions(ZoneModel* zone)
{
	if (mFragsByType.count(0x14) == 0)
		return;

	processMaterials();
	
	//initialize two buffers for each material
	initMaterialBuffers();

	//process mesh fragments one by one
	//0x14 -> 0x2D -> 0x36, 0x14 contains the name
	for (FragHeader* frag : mFragsByType[0x14])
	{
		Frag14* f14 = (Frag14*)frag;
		const char* model_name = getFragName(frag);
		if (f14->size[1] < 1 || model_name == nullptr)
			continue;

		int* ref_ptr = f14->getRefList();
		Frag2D* f2d = (Frag2D*)getFragByRef(*ref_ptr);
		if (f2d == nullptr || f2d->type != 0x2D)
			continue;

		processMesh((Frag36*)getFragByRef(f2d->ref));

		//create irrlicht mesh
		scene::SMesh* mesh = new scene::SMesh;
		scene::SMesh* noncollision_mesh = new scene::SMesh;
		//find any vertex + index buffers with data in them and use them to fill this mesh
		for (uint32 i = 0; i < mNumMaterials; ++i)
		{
			if (!mMaterialVertexBuffers[i].empty())
			{
				createMeshBuffer(mesh, mMaterialVertexBuffers[i], mMaterialIndexBuffers[i], &mMaterials[i], zone);
				mMaterialVertexBuffers[i].clear();
				mMaterialIndexBuffers[i].clear();
			}
			if (!mNoCollisionVertexBuffers[i].empty())
			{
				createMeshBuffer(noncollision_mesh, mNoCollisionVertexBuffers[i], mNoCollisionIndexBuffers[i], &mMaterials[i], zone);
				mNoCollisionVertexBuffers[i].clear();
				mNoCollisionIndexBuffers[i].clear();
			}
		}

		if (mesh->getMeshBufferCount() > 0)
		{
			mesh->recalculateBoundingBox();
			zone->addObjectDefinition(model_name, mesh);
		}

		if (noncollision_mesh->getMeshBufferCount() > 0)
		{
			noncollision_mesh->recalculateBoundingBox();
			zone->addNoCollisionObjectDefinition(model_name, noncollision_mesh);
		}
	}
}

void WLD::convertZoneObjectPlacements(ZoneModel* zone)
{
	if (mFragsByType.count(0x15) == 0)
		return;

	for (FragHeader* frag : mFragsByType[0x15])
	{
		Frag15* f15 = (Frag15*)frag;
		const char* name = getFragName(f15->ref1);
		if (name == nullptr)
			continue;

		ObjectPlacement obj;
		obj.x = f15->pos.x;
		obj.y = f15->pos.z; //irrlicht uses Y for the "up" axis
		obj.z = f15->pos.y;

		obj.rotX = f15->rot.z / 512.0f * 360.0f;
		obj.rotY = -f15->rot.x / 512.0f * 360.0f; //don't even try to make sense of these
		obj.rotZ = f15->rot.y / 512.0f * 360.0f;

		obj.scaleX = f15->scale.z;
		obj.scaleY = f15->scale.y; //scale order is also weird - x is generally not given, assumed to be equal to the others
		obj.scaleZ = f15->scale.z;

		zone->addObjectPlacement(name, obj);
	}
}

void WLD::convertMobModel(const char* id_name)
{
	if (mFragsByType.count(0x14) == 0)
		return;

	processMaterials();

	size_t len = strlen(id_name);

	for (FragHeader* frag : mFragsByType[0x14])
	{
		const char* name = getFragName(frag);
		if (strncmp(id_name, name, len) == 0)
		{
			convertMobModel((Frag14*)frag, id_name);
			return;
		}
	}
}

void WLD::convertAllMobModels()
{
	if (mFragsByType.count(0x14) == 0)
		return;

	processMaterials();

	for (FragHeader* frag : mFragsByType[0x14])
	{
		convertMobModel((Frag14*)frag, getFragName(frag));
	}
}

void WLD::convertMobModel(Frag14* f14, std::string model_id)
{
	if (f14->size[1] < 1)
		return;

	//should handle this separately from zones in a lot of ways
	//should not load all material textures off the bat in case we are only using 1 out of 10 models in a WLD, for one
	
	initMaterialBuffers();

	//0x14 -> 0x11 -> 0x10 -> 0x13 -> 0x12
	//                   | -> 0x2D -> 0x36
	int* ref_ptr = f14->getRefList();
	Frag11* f11 = (Frag11*)getFragByRef(*ref_ptr);
	if (f11->type != 0x11)
		return;
	Frag10* f10 = (Frag10*)getFragByRef(f11->ref);

	//handle skeleton
	scene::SMesh* mesh = new scene::SMesh;

	//the skeleton uses index-based recursion
	Frag10Bone* rootBone = f10->getBoneList();
	Frag10Bone* bone = rootBone;
	std::vector<Frag10Bone*> bones;
	
	for (int i = 0; i < f10->num_bones; ++i)
	{
		bones.push_back(bone);
		bone = bone->getNext();
	}

	WLDSkeleton* skele = new WLDSkeleton(f10->num_bones, mesh);
	//change to animation id numbers later
	std::unordered_map<int, std::unordered_map<std::string, Frag13*, std::hash<std::string>>> animFragsByBone;

	auto findAnimations = [&, this](const char* baseName, int bonePos)
	{
		for (FragHeader* frag : mFragsByType[0x13])
		{
			const char* name = getFragName(frag);
			if (strcmp(name + 3, baseName) != 0)
				continue;

			Frag13* f13 = (Frag13*)frag;
			auto& anims = animFragsByBone[bonePos];
			anims[std::string(name, 3)] = f13;
		}
	};

	//handle root bone
	int* ptr;
	Frag13* f13 = (Frag13*)getFragByRef(rootBone->ref1);
	const char* f13name = getFragName(f13);
	Frag12* f12 = (Frag12*)getFragByRef(f13->ref);
	skele->setBasePosition(0, f12->entry[0]);
	findAnimations(f13name, 0);

	bone = rootBone;
	for (int i = 0; i < f10->num_bones; ++i)
	{
		if (bone->size > 0)
		{
			ptr = bone->getIndexList();
			for (int j = 0; j < bone->size; ++j)
			{
				Frag10Bone* b = bones[*ptr];
				f13 = (Frag13*)getFragByRef(b->ref1);
				f12 = (Frag12*)getFragByRef(f13->ref);
				//check if this is an attachment point
				int point = -1;
				f13name = getFragName(f13);
				size_t len = strlen(f13name);
				if (len > 13 && strcmp(f13name + len - 12, "_POINT_TRACK") == 0)
				{
					const char* c = f13name + len - 13;
					switch (*c)
					{
					case 'R': //right
						point = WLDSkeleton::POINT_RIGHT;
						break;
					case 'L': //left
						point = WLDSkeleton::POINT_LEFT;
						break;
					case 'D': //shielD or heaD
						if (*(c - 1) == 'L')
							point = WLDSkeleton::POINT_SHIELD;
						else
							point = WLDSkeleton::POINT_HEAD;
						break;
					}
				}
				else
				{
					//attachment points never have animation frames
					findAnimations(f13name, *ptr);
				}

				skele->setBasePosition(*ptr++, f12->entry[0], i, point);
			}
		}
		bone = bone->getNext();
	}

	//create animations
	//the root bone's frag13 has the timing information,
	//while its children's frag12s have the number of frames
	for (auto& pair : animFragsByBone[0])
	{
		ptr = rootBone->getIndexList();
		uint32 timing = pair.second->param;

		for (int i = 0; i < rootBone->size; ++i)
		{
			if (animFragsByBone.count(*ptr) && animFragsByBone[*ptr].count(pair.first))
			{
				//we're taking it on faith that the first one we find has more than 1 frame
				f13 = animFragsByBone[*ptr][pair.first];
				f12 = (Frag12*)getFragByRef(f13->ref);
				skele->addAnimation(pair.first, f12->count, timing);
			}
			++ptr;
		}
	}

	//read animations
	//root bone first and separately
	for (auto& pair : animFragsByBone[0])
	{
		f12 = (Frag12*)getFragByRef(pair.second->ref);
		skele->addAnimationFrames(pair.first, f12, 0);
	}

	//now for all child bones
	for (auto& pair : animFragsByBone[0]) //for each animation name
	{
		bone = rootBone;
		for (int i = 0; i < f10->num_bones; ++i)
		{
			if (bone->size > 0)
			{
				ptr = bone->getIndexList();
				for (int j = 0; j < bone->size; ++j)
				{
					f12 = nullptr;
					if (animFragsByBone.count(*ptr) && animFragsByBone[*ptr].count(pair.first))
						f12 = (Frag12*)getFragByRef(animFragsByBone[*ptr][pair.first]->ref);

					skele->addAnimationFrames(pair.first, f12, *ptr++, i);
				}
			}
			bone = bone->getNext();
		}
	}

	//find meshes
	int numMeshes;
	ref_ptr = f10->getRefList(numMeshes);
	for (int n = 0; n < numMeshes; ++n)
	{
		Frag2D* f2d = (Frag2D*)getFragByRef(*ref_ptr++);

		processMesh((Frag36*)getFragByRef(f2d->ref), skele);

		//find any vertex + index buffers with data in them and use them to fill this mesh
		uint32 usedBuffers = 0;
		for (uint32 i = 0; i < mNumMaterials; ++i)
		{
			if (!mMaterialVertexBuffers[i].empty())
			{
				createMeshBuffer(mesh, mMaterialVertexBuffers[i], mMaterialIndexBuffers[i], &mMaterials[i], skele);
				mMaterialVertexBuffers[i].clear();
				mMaterialIndexBuffers[i].clear();

				//need to correct any bone assignment weights to point to the correct buffer index
				for (uint32 j = 0; j < skele->getNumBones(); ++j)
				{
					auto& weights = skele->getWeights(j);
					for (WLDSkeleton::Weight& wt : weights)
					{
						if (wt.buffer_index == i)
							wt.buffer_index = usedBuffers;
					}
				}

				++usedBuffers;
			}
		}

		break; //remove when we can handle head meshes
	}

	gMobMgr.addModelPrototype(Translate::raceID(model_id), Translate::gender(model_id), skele);
}
