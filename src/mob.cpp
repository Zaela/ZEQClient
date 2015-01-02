
#include "mob.h"
#include "renderer.h"
#include "mob_manager.h"

extern Renderer gRenderer;
extern MobManager gMobMgr;

Mob::Mob(uint32 index, WLDSkeleton* skele, MobPosition* pos, WLDSkeleton* head) :
	mIndex(index),
	mHeadNode(nullptr),
	mRightHandNode(nullptr),
	mLeftHandNode(nullptr),
	mShieldNode(nullptr)
{
	initSkeleton(skele, pos, head);
	setName("test");
}

Mob::Mob(Spawn_Struct* spawn, WLDSkeleton* skele, MobPosition* pos, WLDSkeleton* head) :
	mIndex(spawn->spawnId),
	mHeadNode(nullptr),
	mRightHandNode(nullptr),
	mLeftHandNode(nullptr),
	mShieldNode(nullptr)
{
	readSpawnStruct(spawn);
	initSkeleton(skele, pos, head);

	setHeading(Util::unpackHeading(spawn->heading));
}

Mob::Mob(Spawn_Struct* spawn) :
	mIndex(spawn->spawnId),
	mNode(nullptr),
	mSkeletonWLD(nullptr),
	mHeadSkeleWLD(nullptr),
	mHeadNode(nullptr),
	mRightHandNode(nullptr),
	mLeftHandNode(nullptr),
	mShieldNode(nullptr)
{
	readSpawnStruct(spawn);
}

void Mob::readSpawnStruct(Spawn_Struct* spawn)
{
	setName(spawn->name);
	mRace = spawn->race;
	mGender = spawn->gender;
}

void Mob::initSkeleton(WLDSkeleton* skele, MobPosition* pos, WLDSkeleton* head)
{
	mSkeletonWLD = new WLDSkeletonInstance(gRenderer.copyMesh(skele->getReferenceMesh()), skele, this);
	mSkeletonWLD->assumeBasePosition();

	scene::IAnimatedMesh* mesh = new scene::SAnimatedMesh(mSkeletonWLD->getMesh());
	scene::ISceneManager* sceneMgr = gRenderer.getSceneManager();

	mNode = sceneMgr->addAnimatedMeshSceneNode(mesh, nullptr, -1, *pos);
	mesh->drop();

	mInvertHeadingRace = Translate::invertHeadingRace(skele->getModelRace());

	if (skele->hasHeadPoint())
	{
		mHeadNode = sceneMgr->addEmptySceneNode(mNode); //make this a camera scene node later
	}

	if (head)
	{
		mHeadSkeleWLD = new WLDSkeletonInstance(gRenderer.copyMesh(head->getReferenceMesh()), head, this);
		mHeadSkeleWLD->assumeBasePosition();

		mesh = new scene::SAnimatedMesh(mHeadSkeleWLD->getMesh());

		sceneMgr->addAnimatedMeshSceneNode(mesh, mNode);
		mesh->drop();
	}
	else
	{
		mHeadSkeleWLD = nullptr;
	}

	//should be placed above the HEAD_POINT node rather than the base node
	//default font is pretty terrible
	core::stringw wide_name(mDisplayName);
	gRenderer.getSceneManager()->addBillboardTextSceneNode(nullptr, wide_name.c_str(), mNode,
		core::dimension2df(5.0f, 2.0f), core::vector3df(0, 5.0f, 0));
}

Mob::~Mob()
{
	if (mSkeletonWLD)
		delete mSkeletonWLD;
	if (mHeadSkeleWLD)
		delete mHeadSkeleWLD;
	if (mNode)
		mNode->removeAll();
}

void Mob::setName(const char* name)
{
	if (strlen(name) == 0)
		return;
	snprintf(mRawName, 64, "%s", name);
	std::string displayName = Util::getDisplayName(name);
	snprintf(mDisplayName, 64, "%s", displayName.c_str());
}

void Mob::setHeading(float heading)
{
	if (!mInvertHeadingRace)
	{
		if (heading >= 90.0f)
			heading -= 90.0f;
		else
			heading += 270.0f;
	}
	else
	{
		if (heading <= 270.0f)
			heading += 90;
		else
			heading -= 270.0f;
	}

	if(mNode)
		mNode->setRotation(core::vector3df(0.0f, heading, 0.0f));
}

void Mob::animate(float delta)
{
	if (mSkeletonWLD)
	{
		mSkeletonWLD->animate(delta);
		if (mHeadSkeleWLD)
			mHeadSkeleWLD->animate(delta);

		//check point node position and rotation changes
		//head doesn't rotate
	}
}

void Mob::startAnimation(std::string id)
{
	if (mSkeletonWLD)
	{
		mSkeletonWLD->setAnimation(id);
		if (mHeadSkeleWLD)
			mHeadSkeleWLD->setAnimation(id);
	}
}

void Mob::updatePosition(MobPositionUpdate_Struct* update)
{
	//ignoring deltas / interpolation for now...
	MobPosition* cur = gMobMgr.getMobPosition(mIndex);

	core::vector3df pos(
		Util::EQ19toFloat(update->y_pos),
		Util::EQ19toFloat(update->z_pos),
		Util::EQ19toFloat(update->x_pos)
	);

	if (update->delta_x || update->delta_y || update->delta_z)
	{
		//we're walking/running
		//the server doesn't tell us the heading the mob should have while moving - need to calculate based on difference
		float heading = Util::getHeadingTo(*cur, pos);

		/*printf("moving diff: %g, %g, %g - delta: %g, %g, %g\n", cur->X - pos.X, cur->Y - pos.Y, cur->Z - pos.Z,
			Util::EQ13PreciseToFloat(update->delta_x), Util::EQ13PreciseToFloat(update->delta_y),
			Util::EQ13PreciseToFloat(update->delta_z));*/

		*cur = pos;
		if (mNode)
			mNode->setPosition(pos);
		setHeading(heading);
	}
	else
	{
		//we're teleporting
		if(mNode)
			mNode->setPosition(pos);
		setHeading(Util::unpackHeading(update->heading));
	}
}
