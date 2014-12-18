
#include "player.h"

extern Input gInput;
extern Renderer gRenderer;

Player::Player() :
	mCamera(nullptr),
	mMovespeed(100.0f),
	mFallspeed((float)FALLING_SPEED_DEFAULT),
	mIsFalling(true),
	mZoneViewer(nullptr)
{

}

void Player::mainLoop()
{
	for (;;)
	{
		float delta = gRenderer.loopStep();

		mZoneConnection->poll();

		if (gInput.isMoving())
			applyMovement(delta);

		if (mIsFalling)
			applyGravity(delta);
	}
}

void Player::zoneViewerLoop()
{
	for (;;)
	{
		float delta = gRenderer.loopStep();

		if (gInput.isMoving())
			applyMovement(delta);

		if (mZoneViewer->applyGravity && mIsFalling)
			applyGravity(delta);
	}
}

void Player::applyMovement(float delta)
{
	scene::ICameraSceneNode* cam = mCamera->getSceneNode();
	core::vector3df pos = cam->getPosition();

	//update rotation
	core::vector3df target = cam->getTarget() - cam->getAbsolutePosition();
	core::vector3df relativeRotation = target.getHorizontalAngle();

	relativeRotation.Y += gInput.getRelativeMouseXMovement();
	relativeRotation.X += gInput.getRelativeMouseYMovement();

	bool mouseDown = gInput.getRightMouseDown();
	int8 moveDir = gInput.getMoveDirection();
	int8 turnDir = gInput.getTurnDirection();

	if (!mouseDown && turnDir != Input::TURN_NONE)
	{
		relativeRotation.Y += delta * 100 * turnDir; //the 100 corresponds to mouse sensitivity
	}

	target.set(0, 0, core::max_(1.0f, pos.getLength()));

	//avoid gimbal lock, prevent the player from looking quite straight up or down
	//lower half is 0 (mid) to 90 (straight down)
	//upper half is 360 (mid) to 270 (straight up)
	if (relativeRotation.X > 180.0f)
	{
		if (relativeRotation.X < 271.0f)
			relativeRotation.X = 271.0f;
	}
	else
	{
		if (relativeRotation.X > 89.0f)
			relativeRotation.X = 89.0f;
	}

	core::matrix4 mat;
	mat.setRotationDegrees(core::vector3df(relativeRotation.X, relativeRotation.Y, 0));
	mat.transformVect(target);

	core::vector3df movedir = target;
	movedir.normalize();

	if (moveDir != Input::MOVE_NONE)
	{
		pos -= movedir * delta * mMovespeed * moveDir;
	}

	if (mouseDown && turnDir != Input::TURN_NONE)
	{
		core::vector3df strafevect = target;
		strafevect = strafevect.crossProduct(cam->getUpVector());
		strafevect.normalize();

		pos -= strafevect * delta * mMovespeed * turnDir;
	}

	//write translation
	cam->setPosition(pos);

	//write right target
	target += pos;
	cam->setTarget(target);

	mIsFalling = true;
	gInput.resetMoved();
}

void Player::applyGravity(float delta)
{
	scene::ISceneCollisionManager* mgr = gRenderer.getCollisionManager();
	scene::ICameraSceneNode* cam = mCamera->getSceneNode();
	core::vector3df pos = cam->getPosition();

	core::line3df ray;
	ray.start = pos + core::vector3df(0, 5, 0);
	ray.end = pos + core::vector3df(0, -5000, 0);
	core::vector3df collisionPoint;
	core::triangle3df unused;
	scene::ISceneNode* node;

	if (mgr->getCollisionPoint(ray, gRenderer.getCollisionSelector(), collisionPoint, unused, node))
	{
		//a collision was found along the ray
		float min = collisionPoint.Y + 5.0f;
		float yDiff = pos.Y - min;
		if (yDiff > 0.0f)
		{
			//we're falling
			float fallTo = pos.Y - mFallspeed * delta;
			if (fallTo <= min)
			{
				//done falling
				fallTo = min;
				mIsFalling = false;
			}
			yDiff = pos.Y - fallTo;
			pos.Y = fallTo;
			cam->setPosition(pos);
			cam->setTarget(cam->getTarget() - core::vector3df(0, yDiff, 0));
		}
		else
		{
			//we need to correct upwards
			pos.Y -= yDiff;
			cam->setPosition(pos);
			cam->setTarget(cam->getTarget() - core::vector3df(0, yDiff, 0));
			mIsFalling = false;
		}
	}
	else
	{
		//nothing beneath us - don't fall into the void
		mIsFalling = false;
	}
}
