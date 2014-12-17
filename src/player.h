
#ifndef _ZEQ_PLAYER_H
#define _ZEQ_PLAYER_H

#include "types.h"
#include "mob.h"
#include "input.h"
#include "camera.h"
#include "renderer.h"
#include "zone_connection.h"

using namespace irr;

class Player : public Mob
{
private:
	ZoneConnection* mZoneConnection;
	Camera* mCamera;
	float mMovespeed;

public:
	Player();

	void mainLoop();
	void zoneViewerLoop();

	void setZoneConnection(ZoneConnection* zc) { mZoneConnection = zc; }
	void setCamera(Camera* cam) { if (mCamera) delete mCamera; mCamera = cam; }

	void applyMovement(float delta);
};

#endif
