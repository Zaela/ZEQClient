
#ifndef _ZEQ_INPUT_H
#define _ZEQ_INPUT_H

#include <irrlicht.h>

#include "types.h"

using namespace irr;

class Input : public IEventReceiver
{
private:
	scene::ICameraSceneNode* mCamera;

private:
	bool handleKeyboardEvent(const SEvent::SKeyInput& ev);
	bool handleMouseEvent(const SEvent::SMouseInput& ev);

public:
	Input();

	void setCamera(scene::ICameraSceneNode* cam) { mCamera = cam; }

	virtual bool OnEvent(const SEvent& ev) override;
};

#endif
