
#ifndef _ZEQ_INPUT_H
#define _ZEQ_INPUT_H

#include <irrlicht.h>
#include "rocket.h"

#include "types.h"
#include "exception.h"
#include "zone_viewer.h"
#include "structs_mob.h"

using namespace irr;

class Input : public IEventReceiver
{
private:
	bool mSuspendInput;
	int8 mMoveDirection;
	int8 mTurnDirection;
	bool mRightMouseDown;
	int32 mMouseX;
	int32 mMouseY;
	float mRelativeMouseX;
	float mRelativeMouseY;

	Rocket::Core::Context* mGUIContext;

	int mMode;

public:
	enum Movement
	{
		MOVE_FORWARD = -1,
		MOVE_NONE,
		MOVE_BACKWARD
	};

	enum Turn
	{
		TURN_LEFT = -1,
		TURN_NONE,
		TURN_RIGHT
	};

	enum Mode
	{
		ZONE,
		ZONE_VIEWER
	};

	enum RocketMouseButton
	{
		MOUSE_LEFT = 0,
		MOUSE_RIGHT = 1,
		MOUSE_MIDDLE = 2
	};

	enum RocketKeyModifier
	{
		KM_CTRL = 1 << 0,	// Set if at least one Ctrl key is depressed.
		KM_SHIFT = 1 << 1,	// Set if at least one Shift key is depressed.
		KM_ALT = 1 << 2,	// Set if at least one Alt key is depressed.
		KM_META = 1 << 3,	// Set if at least one Meta key (the command key) is depressed.
		KM_CAPSLOCK = 1 << 4,	// Set if caps lock is enabled.
		KM_NUMLOCK = 1 << 5,	// Set if num lock is enabled.
		KM_SCROLLLOCK = 1 << 6	// Set if scroll lock is enabled.
	};

private:
	bool handleKeyboardEvent(const SEvent::SKeyInput& ev);
	bool handleMouseEvent(const SEvent::SMouseInput& ev);

	bool zoneKeyboardEvent(const SEvent::SKeyInput& ev);
	bool zoneMouseEvent(const SEvent::SMouseInput& ev);

	bool zoneViewerKeyboardEvent(const SEvent::SKeyInput& ev);
	bool zoneViewerMouseEvent(const SEvent::SMouseInput& ev);

	int getKeyModifier(const SEvent::SMouseInput& ev);

public:
	Input();

	void setMode(Mode mode) { mMode = mode; }

	void suspendInput(bool toggle) { mSuspendInput = toggle; }

	int8 getMoveDirection() { return mMoveDirection; }
	int8 getTurnDirection() { return mTurnDirection; }
	bool getRightMouseDown() { return mRightMouseDown; }
	float getRelativeMouseXMovement() { return mRelativeMouseX; }
	float getRelativeMouseYMovement() { return mRelativeMouseY; }

	bool isMoving() { return mMoveDirection || mTurnDirection || mRelativeMouseX != 0.0f || mRelativeMouseY != 0.0f; }
	void resetMoved() { mRelativeMouseX = 0; mRelativeMouseY = 0; }

	void setGUIContext(Rocket::Core::Context* cxt) { mGUIContext = cxt; }

	virtual bool OnEvent(const SEvent& ev) override;
};

#endif
