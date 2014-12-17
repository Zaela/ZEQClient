
#include "input.h"

using namespace irr;

Input::Input() :
	mSuspendInput(false),
	mMoveDirection(MOVE_NONE), mTurnDirection(TURN_NONE),
	mRightMouseDown(false), mRelativeMouseX(0), mRelativeMouseY(0)
{

}

bool Input::OnEvent(const SEvent& ev)
{
	if (mSuspendInput)
		return true;

	switch (ev.EventType)
	{
	case EET_KEY_INPUT_EVENT:
	{
		return handleKeyboardEvent(ev.KeyInput);
	}
	case EET_MOUSE_INPUT_EVENT:
	{
		return handleMouseEvent(ev.MouseInput);
	}
	default:
		break;
	}

	return false;
}

bool Input::handleKeyboardEvent(const SEvent::SKeyInput& ev)
{
	switch (ev.Key)
	{
	case KEY_UP:
		if (ev.PressedDown)
		{
			mMoveDirection = MOVE_FORWARD;
		}
		else if (mMoveDirection == MOVE_FORWARD)
		{
			mMoveDirection = MOVE_NONE;
		}
		break;
	case KEY_DOWN:
		if (ev.PressedDown)
		{
			mMoveDirection = MOVE_BACKWARD;
		}
		else if (mMoveDirection == MOVE_BACKWARD)
		{
			mMoveDirection = MOVE_NONE;
		}
		break;
	case KEY_LEFT:
		if (ev.PressedDown)
		{
			mTurnDirection = TURN_LEFT;
		}
		else if (mTurnDirection == TURN_LEFT)
		{
			mTurnDirection = TURN_NONE;
		}
		break;
	case KEY_RIGHT:
		if (ev.PressedDown)
		{
			mTurnDirection = TURN_RIGHT;
		}
		else if (mTurnDirection == TURN_RIGHT)
		{
			mTurnDirection = TURN_NONE;
		}
		break;
	case KEY_ESCAPE:
		throw ExitException();
	default:
		break;
	}

	return false;
}

bool Input::handleMouseEvent(const SEvent::SMouseInput& ev)
{
	switch (ev.Event)
	{
	case EMIE_RMOUSE_PRESSED_DOWN:
		mRightMouseDown = true;
		break;
	case EMIE_RMOUSE_LEFT_UP:
		mRightMouseDown = false;
		break;
	case EMIE_MOUSE_MOVED:
		if (mRightMouseDown)
		{
			mRelativeMouseX += float(ev.X - mMouseX);
			mRelativeMouseY += float(ev.Y - mMouseY);
		}
		mMouseX = ev.X;
		mMouseY = ev.Y;
		break;
	default:
		break;
	}

	return false;
}
