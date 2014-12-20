
#include "wld.h"

extern Renderer gRenderer;

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

void WLD::processMesh(Frag36* f36, scene::CSkinnedMesh* skele)
{
	byte* data = (byte*)f36;
	uint32 p = sizeof(Frag36);

	const float scale = 1.0f / (1 << f36->scale);

	//for bone assignments
	std::vector<uint16> vertToBoneAssignment;
	core::array<scene::CSkinnedMesh::SJoint*>* jointArray = nullptr;
	if (skele)
		jointArray = &skele->getAllJoints();

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
	if (jointArray)
	{
		printf("vert count: %u\n", f36->vert_count);
		WLD_Structs::BoneAssignment* ba = (WLD_Structs::BoneAssignment*)(data + p);
		for (uint16 b = 0; b < f36->bone_assignment_count; ++b)
		{
			printf("\tbone %u count %u\n", ba->index, ba->count);
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

			if (jointArray)
			{
				scene::CSkinnedMesh::SJoint* joint = (*jointArray)[vertToBoneAssignment[idx]];
				scene::ISkinnedMesh::SWeight* wt = skele->addWeight(joint);
				wt->buffer_id = mat_index;
				wt->strength = 1.0f;
				wt->vertex_id = buf_pos;
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
			if ((tri.flag & RawTriangle::PERMEABLE) == 0 || skele)
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

		obj.rotX = f15->rot.y / 512.0f * 360.0f;
		obj.rotY = -f15->rot.x / 512.0f * 360.0f; //don't even try to make sense of these
		obj.rotZ = f15->rot.z / 512.0f * 360.0f;

		obj.scaleX = f15->scale.z;
		obj.scaleY = f15->scale.y; //scale order is also weird - x is generally not given, assumed to be equal to the others
		obj.scaleZ = f15->scale.z;

		zone->addObjectPlacement(name, obj);
	}
}

void WLD::createSkinnedMeshBuffer(scene::CSkinnedMesh* mesh, std::vector<video::S3DVertex>& vert_buf,
		std::vector<uint32>& index_buf, IntermediateMaterial* mat)
{
	scene::SSkinMeshBuffer* mesh_buffer = mesh->addMeshBuffer();
	auto& vbuf = mesh_buffer->Vertices_Standard;
	auto& ibuf = mesh_buffer->Indices;

	uint32 count = index_buf.size();
	for (uint32 j = 0; j < count; ++j)
	{
		vbuf.push_back(vert_buf[j]);
		ibuf.push_back((uint16)(index_buf[j]));
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
			//zone->addUsedTexture(mat->first.diffuse_map); //make a general Model class for this

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
}

MobModel* WLD::convertMobModel(const char* id_name)
{
	if (mFragsByType.count(0x14) == 0)
		return nullptr;

	processMaterials();

	size_t len = strlen(id_name);

	for (FragHeader* frag : mFragsByType[0x14])
	{
		const char* name = getFragName(frag);
		if (strncmp(id_name, name, len) == 0)
		{
			return convertMobModel((Frag14*)frag);
		}
	}

	return nullptr;
}

MobModel* WLD::convertMobModel(Frag14* f14)
{
	if (f14->size[1] < 1)
		return nullptr;

	//should handle this separately from zones in a lot of ways
	//should not load all material textures off the bat in case we are only using 1 out of 10 models in a WLD, for one
	
	//process bones to get all the joints in order (check for and record attachment points!)
	//then process the 0x36s to get the bone assignments
	//and maybe also do the 0x30 --> 0x03 part there too if you can avoid making a mess

	//should flag chr wld's and remove hardware textures on their destructors ?
	
	//initialize two buffers for each material
	initMaterialBuffers();

	//0x14 -> 0x11 -> 0x10 -> 0x13 -> 0x12
	//                   | -> 0x2D -> 0x36
	int* ref_ptr = f14->getRefList();
	Frag11* f11 = (Frag11*)getFragByRef(*ref_ptr);
	if (f11->type != 0x11)
		return nullptr;
	Frag10* f10 = (Frag10*)getFragByRef(f11->ref);

	MobModel* mob = new MobModel;
	//try doing the manual rotate and shift by parent for each keyframe of each joint

	//handle skeleton here
	scene::CSkinnedMesh* mainMesh = new scene::CSkinnedMesh;

	//the skeleton uses index-based recursion
	Frag10Bone* rootBone = f10->getBoneList();
	
	//first, create the individual bones
	/*auto processBone = [=](Frag10Bone* f10bone, scene::CSkinnedMesh::SJoint* joint)
	{
		Frag13* f13 = (Frag13*)getFragByRef(f10bone->ref1);

		const char* name = getFragName(f13);
		printf("%s\n", name);

		findAnimations(name, joint);

		joint->Name = name;

		Frag12* f12 = (Frag12*)getFragByRef(f13->ref);
		Frag12Entry& ent = f12->entry[0];

		float denom = (float)ent.rotDenom;
		core::vector3df rot(
			(float)ent.rotX / denom * 3.14159f / 180.0f,
			(float)ent.rotZ / denom * 3.14159f / 180.0f,
			(float)ent.rotY / denom * 3.14159f / 180.0f
		);

		joint->Animatedrotation = core::quaternion(rot);

		denom = (float)ent.shiftDenom;
		joint->Animatedposition = core::vector3df(
			(float)ent.shiftX / denom,
			(float)ent.shiftZ / denom,
			(float)ent.shiftY / denom
		);

		joint->Animatedscale = core::vector3df(1.0f, 1.0f, 1.0f);

		core::matrix4 positionMatrix;
		positionMatrix.setTranslation(joint->Animatedposition);
		core::matrix4 scaleMatrix;
		scaleMatrix.setScale(joint->Animatedscale);
		core::matrix4 rotationMatrix;

		joint->Animatedrotation.getMatrix_transposed(rotationMatrix);

		joint->LocalMatrix = positionMatrix * rotationMatrix * scaleMatrix;
		//set global matrix during next step, when we know the parent for this bone
	};*/

	Frag10Bone* bone = rootBone;
	std::vector<core::vector3df> rotationsVec;
	for (int i = 0; i < f10->num_bones; ++i)
	{
		//processBone(bone, mainMesh->addJoint());
		scene::CSkinnedMesh::SJoint* joint = mainMesh->addJoint();

		Frag13* f13 = (Frag13*)getFragByRef(bone->ref1);

		const char* name = getFragName(f13);
		printf("%s\n", name);

		findAnimations(name, joint);

		joint->Name = name;

		Frag12* f12 = (Frag12*)getFragByRef(f13->ref);
		Frag12Entry& ent = f12->entry[0];

		float denom = (float)ent.rotDenom;
		core::vector3df rot(
			(float)ent.rotX / denom * 3.14159f / 180.0f,
			(float)ent.rotZ / denom * 3.14159f / 180.0f,
			(float)ent.rotY / denom * 3.14159f / 180.0f
		);

		rotationsVec.push_back(rot);
		joint->Animatedrotation = core::quaternion(rot);

		denom = (float)ent.shiftDenom;
		joint->Animatedposition = core::vector3df(
			(float)ent.shiftX / denom,
			(float)ent.shiftZ / denom,
			(float)ent.shiftY / denom
		);

		joint->Animatedscale = core::vector3df(1.0f, 1.0f, 1.0f);

		core::matrix4 positionMatrix;
		positionMatrix.setTranslation(joint->Animatedposition);
		core::matrix4 scaleMatrix;
		scaleMatrix.setScale(joint->Animatedscale);
		core::matrix4 rotationMatrix;

		joint->Animatedrotation.getMatrix_transposed(rotationMatrix);

		joint->LocalMatrix = positionMatrix * rotationMatrix * scaleMatrix;
		//set global matrix during next step, when we know the parent for this bone

		bone = bone->getNext();
	}

	//handle parent -> child relationships
	bone = rootBone;
	//the root bone has no parent
	core::array<scene::CSkinnedMesh::SJoint*>& jointArray = mainMesh->getAllJoints();
	jointArray[0]->GlobalMatrix = jointArray[0]->LocalMatrix;
	for (int i = 0; i < f10->num_bones; ++i)
	{
		if (bone->size > 0)
		{
			scene::CSkinnedMesh::SJoint* parent = jointArray[i];
			core::vector3df& parentRot = rotationsVec[i];
			int* index_ptr = bone->getIndexList();
			for (int j = 0; j < bone->size; ++j)
			{
				scene::CSkinnedMesh::SJoint* child = jointArray[*index_ptr];

				core::vector3df& rot = rotationsVec[*index_ptr++];
				Util::rotateBy(child->Animatedposition, parentRot);

				child->Animatedposition += parent->Animatedposition;
				rot = parentRot - rot;

				child->Animatedrotation = core::quaternion(rot);

				core::matrix4 positionMatrix;
				positionMatrix.setTranslation(child->Animatedposition);
				core::matrix4 scaleMatrix;
				scaleMatrix.setScale(child->Animatedscale);
				core::matrix4 rotationMatrix;

				child->Animatedrotation.getMatrix_transposed(rotationMatrix);

				child->LocalMatrix = positionMatrix * rotationMatrix * scaleMatrix;
				child->GlobalMatrix = child->LocalMatrix;

				parent->Children.push_back(child);
			}
		}
		bone = bone->getNext();
	}

	//build animations
	scene::CSkinnedMesh::SJoint* root = jointArray[0];
	uint32 high_frame = 0;
	for (auto& pair : mAnimFragsByJoint[root])
	{
		const char* animName = pair.first.c_str();
		buildAnimation(animName, mainMesh, root, mob, high_frame);
		for (uint32 i = 0; i < root->Children.size(); ++i)
			buildAnimation(animName, mainMesh, root->Children[i], mob, high_frame, root);
		//for (uint32 i = 1; i < jointArray.size(); ++i)
		//	buildAnimation(animName, mainMesh, jointArray[i], mob, high_frame);
	}

	//find meshes
	int numMeshes;
	ref_ptr = f10->getRefList(numMeshes);
	for (int n = 0; n < numMeshes; ++n)
	{
		Frag2D* f2d = (Frag2D*)getFragByRef(*ref_ptr++);

		processMesh((Frag36*)getFragByRef(f2d->ref), mainMesh);

		//find any vertex + index buffers with data in them and use them to fill this mesh
		uint32 usedBuffers = 0;
		for (uint32 i = 0; i < mNumMaterials; ++i)
		{
			if (!mMaterialVertexBuffers[i].empty())
			{
				createSkinnedMeshBuffer(mainMesh, mMaterialVertexBuffers[i], mMaterialIndexBuffers[i], &mMaterials[i]);
				mMaterialVertexBuffers[i].clear();
				mMaterialIndexBuffers[i].clear();
				//need to correct any bone assignment weights to point to the correct buffer index
				//do this better, way too much wasted iteration
				for (uint32 j = 0; j < jointArray.size(); ++j)
				{
					auto& weights = jointArray[j]->Weights;
					for (uint32 w = 0; w < weights.size(); ++w)
					{
						scene::ISkinnedMesh::SWeight& wt = weights[w];
						if (wt.buffer_id == i)
							wt.buffer_id = usedBuffers;
					}
				}

				++usedBuffers;
			}
		}

		mainMesh->finalize();
		mob->setMesh(n, mainMesh);

		//
		break;
	}

	return mob;
}

void WLD::findAnimations(const char* baseName, scene::CSkinnedMesh::SJoint* joint)
{
	for (FragHeader* frag : mFragsByType[0x13])
	{
		const char* name = getFragName(frag);
		if (strcmp(name + 3, baseName) != 0)
			continue;

		printf("joint %p found f13 %s\n", joint, name);
		Frag13* f13 = (Frag13*)frag;
		auto& anims = mAnimFragsByJoint[joint];
		anims[std::string(name, 3)] = f13;
	}
}

void WLD::buildAnimation(const char* animName, scene::CSkinnedMesh* skele, scene::CSkinnedMesh::SJoint* joint, 
	MobModel* mob, uint32& high_frame, scene::CSkinnedMesh::SJoint* parent)
{
	Frag13* f13 = mAnimFragsByJoint[joint][animName];
	if (f13 == nullptr)
		return;

	Frag12* f12 = (Frag12*)getFragByRef(f13->ref);


	Animation* anim = mob->getAnimation(animName);
	if (parent == nullptr) //new animation, root bone
	{
		anim->frame_delay = f13->param;
		anim->start_frame = high_frame;
		printf("%s delay: %u, start_frame: %u\n", animName, f13->param, high_frame);
	}
	else if (anim->end_frame == 0)
	{
		uint32 frames_len = anim->frame_delay * f12->count;
		anim->end_frame = high_frame + frames_len;
		high_frame += frames_len + 1;
		printf("%s end_frame: %u\n", animName, anim->end_frame);
	}

	//create keyframes
	float frame = (float)anim->start_frame;

	for (uint32 i = 0; i < f12->count; ++i)
	{
		Frag12Entry& ent = f12->entry[i];

		float denom = (float)ent.rotDenom;
		core::vector3df rot(
			(float)ent.rotX / denom * 3.14159f / 180.0f,
			(float)ent.rotZ / denom * 3.14159f / 180.0f,
			(float)ent.rotY / denom * 3.14159f / 180.0f
		);

		denom = (float)ent.shiftDenom;
		scene::ISkinnedMesh::SPositionKey* pos = skele->addPositionKey(joint);
		pos->frame = frame;
		pos->position = core::vector3df(
			(float)ent.shiftX / denom,
			(float)ent.shiftZ / denom,
			(float)ent.shiftY / denom
		);

		if (parent)
		{
			core::vector3df parentPos;
			core::vector3df parentRot;
			//uint32 k = parent->PositionKeys.size() > i ? i : 0;
			uint32 k;
			for (k = 0; k < parent->PositionKeys.size(); ++k)
			{
				if (parent->PositionKeys[k].frame == frame)
					break;
			}
			parentPos = parent->PositionKeys[k].position;
			parent->RotationKeys[k].rotation.toEuler(parentRot);

			Util::rotateBy(pos->position, parentRot);
			pos->position += parentPos;
			rot = parentRot - rot;
		}

		scene::ISkinnedMesh::SRotationKey* rotKey = skele->addRotationKey(joint);
		rotKey->frame = frame;
		rotKey->rotation = core::quaternion(rot);

		scene::ISkinnedMesh::SScaleKey* scale = skele->addScaleKey(joint);
		scale->frame = frame;
		scale->scale = core::vector3df(1.0f, 1.0f, 1.0f);

		frame += (float)anim->frame_delay;
	}

	for (uint32 i = 0; i < joint->Children.size(); ++i)
		buildAnimation(animName, skele, joint->Children[i], mob, high_frame, joint);
}
