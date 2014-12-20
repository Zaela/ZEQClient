
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

void Player::setCamera(Camera* cam)
{
	if (mCamera)
		delete mCamera;
	mCamera = cam;
	//REPLACE THIS - crappy irrlicht collision thing for now
	/*scene::ISceneNodeAnimatorCollisionResponse* acr = gRenderer.getSceneManager()->createCollisionResponseAnimator(
		gRenderer.getCollisionSelector(),
		cam->getSceneNode(), core::vector3df(1.5f, 3.0f, 1.5f), core::vector3df(0.0f, 0.0f, 0.0f),
		core::vector3df(0.0f, 3.0f, 0.0f), 0.0f);
	cam->getSceneNode()->addAnimator(acr);*/
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

	core::vector3df dest = pos;
	if (moveDir != Input::MOVE_NONE)
	{
		dest -= movedir * delta * mMovespeed * moveDir;
	}

	if (mouseDown && turnDir != Input::TURN_NONE)
	{
		core::vector3df strafevect = target;
		strafevect = strafevect.crossProduct(cam->getUpVector());
		strafevect.normalize();

		dest -= strafevect * delta * mMovespeed * turnDir;
	}

	//check collision between pos and dest
	if (!mZoneViewer || mZoneViewer->applyCollision)
		checkCollision(pos, dest);

	//write translation
	cam->setPosition(dest);

	//write right target
	target += dest;
	cam->setTarget(target);

	mIsFalling = true;
	mFallStartingY = pos.Y;
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

	if (mgr->getSceneNodeAndCollisionPointFromRay(ray, collisionPoint, unused))
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
				applyFallingDamage();
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

void Player::checkCollision(core::vector3df& from, core::vector3df& dest)
{
	//just like gravity in another direction, really
	//just need to get a vector from a -> b by vector subtraction
	//normalize it to unit length and multiply it to the desired length
	//then shoot your ray and do the same stuff with the collision point, if any
	/*scene::ISceneCollisionManager* mgr = gRenderer.getCollisionManager();

	core::line3df ray;
	ray.start = from + core::vector3df(0, 5, 0);
	ray.end = dest + core::vector3df(0, 5, 0);
	core::vector3df collisionPoint;
	core::triangle3df unused;

	if (mgr->getSceneNodeAndCollisionPointFromRay(ray, collisionPoint, unused))
	{
		printf("collision: %g, %g, %g to %g, %g, %g\n", from.X, from.Y, from.Z, dest.X, dest.Y, dest.Z);
		core::vector3df diff = from - collisionPoint;
		diff.normalize();
		dest = collisionPoint - core::vector3df(0, 5, 0) - diff;
		printf("corrected to %g, %g, %g\n", dest.X, dest.Y, dest.Z);
	}*/
}

void Player::applyFallingDamage()
{
	float dist = mFallStartingY - mCamera->getSceneNode()->getPosition().Y; //change this
	//threshold and damage calculation, I have no idea at the moment
}
