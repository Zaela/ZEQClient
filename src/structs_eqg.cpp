
#include "structs_eqg.h"
#include "ter.h"
#include "renderer.h"

extern Renderer gRenderer;

static std::unordered_map<std::string, EQG_Structs::PropertyFunction, std::hash<std::string>> PropertyFunctions;

namespace EQG_Structs
{
	std::unordered_map<std::string, PropertyFunction, std::hash<std::string>>& getPropertyFunctions()
	{
		return PropertyFunctions;
	}

	void initialize()
	{
		PropertyFunctions["e_TextureDiffuse0"] = e_TextureDiffuse0;
	}

	PROP_FUNC(e_TextureDiffuse0)
	{
		const char* name = &ter->getStringBlock()[prop->value.i];
		S3D* s3d = ter->getContainingS3D();

		MemoryStream* file = s3d->getFile(name);
		if (file == nullptr)
			return;
		std::string texname = ter->getShortName();
		texname += '/';
		texname += name;
		bool isDDS = false;

		mat_ent.diffuse_map = gRenderer.createTexture(file, name, isDDS);
	}
}
