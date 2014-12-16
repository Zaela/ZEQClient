
#include "wld.h"

extern Renderer gRenderer;

WLD::WLD(MemoryStream* mem, S3D* s3d, std::string shortname) :
	mShortName(shortname),
	mContainingS3D(s3d),
	mNumMaterials(0),
	mMaterials(nullptr)
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

	mNumMaterials = mFragsByType[0x30].size();

	mMaterials = new IntermediateMaterial[mNumMaterials];

	int i = -1;
	for (FragHeader* frag : mFragsByType[0x30])
	{
		Frag30* f30 = (Frag30*)frag;
		Frag03* f03;

		IntermediateMaterial* mat = &mMaterials[++i];
		mat->first.diffuse_map = nullptr;
		mat->first.normal_map = nullptr;
		mat->additional = nullptr;

		mMaterialsByFrag30[f30] = mat;
		
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
		mat_ent->normal_map = nullptr;
	}
}

uint32 WLD::translateVisibilityFlag(Frag30* f30, bool isDDS)
{
	uint32 ret = 0;

	if (f30->flag & Frag30::MASKED)
		ret |= IntermediateMaterialEntry::MASKED;
	if (f30->flag & Frag30::SEMI_TRANSPARENT)
		ret |= IntermediateMaterialEntry::SEMI_TRANSPARENT;
	if (isDDS)
		ret |= IntermediateMaterialEntry::DDS_TEXTURE;

	return ret;
}

void WLD::processMesh(Frag36* f36)
{
	byte* data = (byte*)f36;
	uint32 p = sizeof(Frag36);

	const float scale = 1.0f / (1 << f36->scale);

	//raw vertices
	RawVertex* wld_verts = (RawVertex*)(data + p);
	p += sizeof(RawVertex) * f36->vert_count;
}

scene::IAnimatedMesh* WLD::convertZoneGeometry()
{
	if (mFragsByType.count(0x36) == 0)
		return nullptr;

	processMaterials();
	
	//initialize two buffers for each material
	for (uint32 i = 0; i < mNumMaterials; ++i)
	{
		//placement new
		new (&mMaterialVertexBuffers[i]) std::vector<video::S3DVertex>;
		new (&mMaterialIndexBuffers[i]) std::vector<uint32>;
	}
	//should also make a collision mesh buffer here...
	
	//process mesh fragments
	for (FragHeader* frag : mFragsByType[0x36])
	{
		processMesh((Frag36*)frag);
	}

	return nullptr;
}
