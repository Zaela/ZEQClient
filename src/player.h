
#ifndef _ZEQ_PLAYER_H
#define _ZEQ_PLAYER_H

#include <irrlicht.h>

#include "types.h"
#include "mob.h"
#include "input.h"
#include "camera.h"
#include "renderer.h"
#include "zone_connection.h"
#include "zone_viewer.h"

using namespace irr;

struct MobPosition;

class Player
{
private:
	ZoneConnection* mZoneConnection;
	Camera* mCamera;
	float mMovespeed;
	float mFallspeed;
	bool mIsFalling;
	float mFallStartingY;

	Mob* mMob;
	ZoneViewerData* mZoneViewer;

private:
	//constants
	static const uint32 FALLING_SPEED_DEFAULT = 125;
	static const uint32 FALLING_SPEED_SWIMMING = 25;

private:
	void applyMovement(float delta);
	void applyGravity(float delta);
	void applyFallingDamage();
	void checkCollision(core::vector3df& from, core::vector3df& dest);

public:
	Player();

	void mainLoop();
	void zoneViewerLoop();

	ZoneViewerData* getZoneViewer() { return mZoneViewer; }
	void setZoneViewer(ZoneViewerData* data) { mZoneViewer = data; }

	void setZoneConnection(ZoneConnection* zc) { mZoneConnection = zc; }
	void setCamera(Camera* cam);

	int getEntityID() { return -1; } //put this in later

	void getCoords(MobPosition& pos);

	void handleSpawn(Spawn_Struct* spawn);
	void handlePlayerProfile(PlayerProfile_Struct* pp);
};

#endif
