
#ifndef _ZEQ_STRUCTS_EQG_H
#define _ZEQ_STRUCTS_EQG_H

#include <string>
#include <unordered_map>

#include "types.h"
#include "util.h"
#include "structs_intermediate.h"

class ModelSource;

namespace EQG_Structs
{
	struct Property;

	typedef void(*PropertyFunction)(Property*, IntermediateMaterialEntry&, ModelSource*);
#define PROP_FUNC(x) void x(Property* prop, IntermediateMaterialEntry& mat_ent, ModelSource* modelSource)

	std::unordered_map<std::string, PropertyFunction, std::hash<std::string>>& getPropertyFunctions();
	void initialize();
	PROP_FUNC(e_TextureDiffuse0);

	struct Material
	{
		uint32 index;
		uint32 name_index;
		uint32 shader_index;
		uint32 property_count;
	};

	struct Property
	{
		uint32 name_index;
		uint32 type;
		union
		{
			uint32 i;
			float f;
		} value;
	};

	struct Vertex
	{
		float x, y, z;
		float i, j, k;
		float u, v;
	};

	struct VertexV3
	{
		float x, y, z;
		float i, j, k;
		uint32 color;
		float u, v;
		float unknown[2];
	};

	struct Triangle
	{
		enum Flags
		{
			PERMEABLE = 0x01 //not subject to collision
		};

		uint32 index[3];
		int material;
		uint32 flag;
	};

	struct Bone
	{
		uint32 name_index;
		uint32 link_bone;
		uint32 flag;
		uint32 child_bone;
		float x, y, z;
		float rotX, rotY, rotZ, rotW;
		float scaleX, scaleY, scaleZ;
	};

	struct BoneWeight
	{
		int bone;
		float weight;
	};

	struct BoneAssignment
	{
		uint32 count; //max 4
		BoneWeight weights[4];
	};
}

#endif
