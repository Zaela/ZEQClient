
#include "zone_model.h"
#include "renderer.h"
#include "file_loader.h"
#include "wld.h"
#include "zon.h"
#include "ter.h"

extern Renderer gRenderer;
extern FileLoader gFileLoader;

ZoneModel::ZoneModel() :
	mX(0), mY(0), mZ(0),
	mMesh(nullptr),
	mNonCollisionMesh(nullptr)
{

}

ZoneModel::~ZoneModel()
{
	if (mMesh)
		mMesh->drop();
	if (mNonCollisionMesh)
		mNonCollisionMesh->drop();

	for (auto& pair : mObjectDefinitions)
		pair.second->drop();
}

void ZoneModel::setMeshes(scene::SMesh* mesh, scene::SMesh* nocollide_mesh)
{
	mMesh = new scene::SAnimatedMesh(mesh);
	mesh->drop();
	mNonCollisionMesh = new scene::SAnimatedMesh(nocollide_mesh);
	nocollide_mesh->drop();
}

void ZoneModel::addObjectDefinition(const char* name, scene::SMesh* mesh)
{
	scene::SAnimatedMesh* animMesh = new scene::SAnimatedMesh(mesh);
	mesh->drop();
	mObjectDefinitions[name] = animMesh;
}

void ZoneModel::addNoCollisionObjectDefinition(const char* name, scene::SMesh* mesh)
{
	scene::SAnimatedMesh* animMesh = new scene::SAnimatedMesh(mesh);
	mesh->drop();
	mNoCollisionObjectDefinitions[name] = animMesh;
}

void ZoneModel::addObjectPlacement(const char* name, ObjectPlacement& placement)
{
	if (mObjectDefinitions.count(name) > 0)
	{
		scene::IAnimatedMesh* mesh = mObjectDefinitions[name];
		placement.mesh = mesh;
		placement.collidable = true;

		mObjectPlacements.push_back(placement);
	}

	if (mNoCollisionObjectDefinitions.count(name) > 0)
	{
		scene::IAnimatedMesh* mesh = mNoCollisionObjectDefinitions[name];
		placement.mesh = mesh;
		placement.collidable = false;

		mObjectPlacements.push_back(placement);
	}
}

ZoneModel* ZoneModel::load(std::string shortname)
{
	//try WLD
	WLD* wld = gFileLoader.getWLD(shortname, nullptr, false);
	if (wld)
		return loadFromWLD(shortname, wld);

	//try ZON
	ZON* zon = gFileLoader.getZON(shortname);
	if (zon)
		return loadFromZON(shortname, zon);

	return nullptr;
}

ZoneModel* ZoneModel::loadFromWLD(std::string shortname, WLD* wld)
{
	ZoneModel* zoneModel = wld->convertZoneModel();

	WLD* objWLD = gFileLoader.getWLD(shortname + "_obj", nullptr, false);
	if (objWLD)
	{
		objWLD->convertZoneObjectDefinitions(zoneModel);
		delete objWLD;
	}

	WLD* placeWLD = gFileLoader.getWLD("objects", shortname.c_str(), false);
	if (placeWLD)
	{
		placeWLD->convertZoneObjectPlacements(zoneModel);
		delete placeWLD;
	}

	delete wld;

	gFileLoader.unloadS3D(shortname);
	gFileLoader.unloadS3D(shortname + "_obj");

	return zoneModel;
}

ZoneModel* ZoneModel::loadFromZON(std::string shortname, ZON* zon)
{
	TER* ter = zon->getTER();
	if (ter == nullptr)
	{
		delete zon;
		return nullptr;
	}

	ZoneModel* zoneModel = ter->convertZoneModel();


	delete ter;
	delete zon;

	gFileLoader.unloadS3D(shortname);

	return zoneModel;
}
