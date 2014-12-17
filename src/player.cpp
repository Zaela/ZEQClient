
#include "player.h"

extern Input gInput;
extern Renderer gRenderer;

Player::Player() :
	mCamera(nullptr),
	mMovespeed(100.0f)
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
	}
}

void Player::zoneViewerLoop()
{
	for (;;)
	{
		float delta = gRenderer.loopStep();

		if (gInput.isMoving())
			applyMovement(delta);
	}
}

void Player::applyMovement(float delta)
{
	scene::ICameraSceneNode* cam = mCamera->getSceneNode();
	core::vector3df pos = cam->getPosition();

	// Update rotation
	core::vector3df target = (cam->getTarget() - cam->getAbsolutePosition());
	core::vector3df relativeRotation = target.getHorizontalAngle();

	relativeRotation.Y += gInput.getRelativeMouseXMovement();
	relativeRotation.X += gInput.getRelativeMouseYMovement();

	bool mouseDown = gInput.getRightMouseDown();
	int8 moveDir = gInput.getMoveDirection();
	int8 turnDir = gInput.getTurnDirection();

	if (!mouseDown && turnDir != Input::TURN_NONE)
	{
		relativeRotation.Y += delta * 100 * turnDir;
	}

	target.set(0, 0, core::max_(1.f, pos.getLength()));

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

	// write translation
	cam->setPosition(pos);

	// write right target
	target += pos;
	cam->setTarget(target);

	gInput.resetMoved();
}
