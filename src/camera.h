
#ifndef _ZEQ_CAMERA_H
#define _ZEQ_CAMERA_H

#include <irrlicht.h>

#include "types.h"

using namespace irr;

class Camera
{
private:
	scene::ICameraSceneNode* mCamera;

public:
	Camera(scene::ICameraSceneNode* cam);

	scene::ICameraSceneNode* getSceneNode() { return mCamera; }
};

#endif
