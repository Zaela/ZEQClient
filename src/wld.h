
#ifndef _ZEQ_WLD_H
#define _ZEQ_WLD_H

#include <irrlicht.h>

#include <vector>
#include <unordered_map>
#include <cstring>

#include "types.h"
#include "util.h"
#include "structs_wld.h"
#include "structs_intermediate.h"
#include "memory_stream.h"
#include "exception.h"
#include "s3d.h"
#include "renderer.h"
#include "buffer.h"

using namespace WLD_Structs;

class WLD
{
private:
	struct Header
	{
		char magic[4];
		uint32 version;
		uint32 frag_count;
		uint32 unknownA[2];
		uint32 strings_len;
		uint32 unknownB;
		static const uint32 VERSION1 = 0x00015500;
		static const uint32 VERSION2 = 0x1000C800;
	};

	Header* mHeader;
	int mVersion;
	const char* mStringBlock;
	std::string mShortName;
	S3D* mContainingS3D;

	std::vector<FragHeader*> mFragsByIndex;
	std::unordered_map<uint32, std::vector<FragHeader*>> mFragsByType;
	std::unordered_map<uint32, FragHeader*> mFragsByNameRef;

	/* probably move these off to a common superclass when other file types are used as zone model sources */
	/* --------------------------------------------- */
	uint32 mNumMaterials;
	IntermediateMaterial* mMaterials;

	std::vector<video::S3DVertex>* mMaterialVertexBuffers;
	std::vector<uint32>* mMaterialIndexBuffers;
	std::vector<video::S3DVertex> mCollisionVertexBuffer;
	std::vector<uint32> mCollisionIndexBuffer;
	/* --------------------------------------------- */

	std::unordered_map<Frag30*, int> mMaterialIndicesByFrag30;
	std::unordered_map<Frag03*, const char*> mTexturesByFrag03;

private:
	void processMaterials();
	void Frag03ToMaterialEntry(Frag03* f03, Frag30* f30, IntermediateMaterialEntry* mat_ent);
	void handleAnimatedMaterial(Frag04* f04, Frag30* f30, IntermediateMaterial* mat);
	static uint32 translateVisibilityFlag(Frag30* f30, bool isDDS);
	void processMesh(Frag36* f36);
	
public:
	WLD(MemoryStream* mem, S3D* s3d, std::string shortname);
	
	static void decodeString(void* str, size_t len);

	FragHeader* getFragByRef(int ref);
	const char* getFragName(FragHeader* frag);
	const char* getFragName(int ref);

	uint32 getVersion() { return mVersion; }

	scene::IAnimatedMesh* convertZoneGeometry();
};

#endif
