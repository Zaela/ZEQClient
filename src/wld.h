
#ifndef _ZEQ_WLD_H
#define _ZEQ_WLD_H

#include <irrlicht.h>
#include <CSkinnedMesh.h>

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
#include "zone_model.h"
#include "mob_model.h"
#include "model_source.h"
#include "wld_skeleton.h"
#include "mob_manager.h"

using namespace WLD_Structs;

class WLD : public ModelSource
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

	std::vector<FragHeader*> mFragsByIndex;
	std::unordered_map<uint32, std::vector<FragHeader*>> mFragsByType;
	std::unordered_map<uint32, FragHeader*> mFragsByNameRef;

	std::unordered_map<Frag30*, int> mMaterialIndicesByFrag30;
	std::unordered_map<Frag03*, const char*> mTexturesByFrag03;

	std::unordered_map<scene::CSkinnedMesh::SJoint*, std::unordered_map<std::string, Frag13*, std::hash<std::string>>> mAnimFragsByJoint;

private:
	void processMaterials();
	void Frag03ToMaterialEntry(Frag03* f03, Frag30* f30, IntermediateMaterialEntry* mat_ent);
	void handleAnimatedMaterial(Frag04* f04, Frag30* f30, IntermediateMaterial* mat);
	static uint32 translateVisibilityFlag(Frag30* f30, bool isDDS);
	//void processMesh(Frag36* f36, scene::CSkinnedMesh* skele = nullptr);
	void processMesh(Frag36* f36, WLDSkeleton* skele = nullptr);
	void processTriangle(RawTriangle* tri, uint32 count, std::vector<video::S3DVertex>& vert_buf,
		std::vector<uint32>& index_buf, RawVertex* vert, RawNormal* norm, RawUV16* uv16, RawUV32* uv32);

	//MobModel* convertMobModel(Frag14* f14);
	WLDSkeletonInstance* convertMobModel(Frag14* f14, std::string model_id);
	static void createSkinnedMeshBuffer(scene::CSkinnedMesh* mesh, std::vector<video::S3DVertex>& vert_buf,
		std::vector<uint32>& index_buf, IntermediateMaterial* mat = nullptr);
	void findAnimations(const char* baseName, scene::CSkinnedMesh::SJoint* joint);

	void buildAnimation(const char* animName, scene::CSkinnedMesh* skele, scene::CSkinnedMesh::SJoint* joint, 
		MobModel* mob, uint32& high_frame, scene::CSkinnedMesh::SJoint* parent = nullptr);
	
public:
	WLD(MemoryStream* mem, S3D* s3d, std::string shortname);
	
	static void decodeString(void* str, size_t len);

	FragHeader* getFragByRef(int ref);
	const char* getFragName(FragHeader* frag);
	const char* getFragName(int ref);

	uint32 getVersion() { return mVersion; }

	ZoneModel* convertZoneModel();
	void convertZoneObjectDefinitions(ZoneModel* zone);
	void convertZoneObjectPlacements(ZoneModel* zone);

	//MobModel* convertMobModel(const char* id_name);
	WLDSkeletonInstance* convertMobModel(const char* id_name);
};

#endif
