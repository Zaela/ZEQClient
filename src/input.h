
#ifndef _ZEQ_INPUT_H
#define _ZEQ_INPUT_H

#include <irrlicht.h>
#include "cegui.h"

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

	CEGUI::GUIContext* mGUIContext;

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

private:
	bool handleKeyboardEvent(const SEvent::SKeyInput& ev);
	bool handleMouseEvent(const SEvent::SMouseInput& ev);

	bool zoneKeyboardEvent(const SEvent::SKeyInput& ev);
	bool zoneMouseEvent(const SEvent::SMouseInput& ev);

	bool zoneViewerKeyboardEvent(const SEvent::SKeyInput& ev);
	bool zoneViewerMouseEvent(const SEvent::SMouseInput& ev);

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

	void setGUIContext(CEGUI::GUIContext* cxt) { mGUIContext = cxt; }

	virtual bool OnEvent(const SEvent& ev) override;
};

#endif
