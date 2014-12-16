
#include "input.h"

using namespace irr;

Input::Input() :
	mCamera(nullptr)
{

}

bool Input::OnEvent(const SEvent& ev)
{
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
	return false;
}

bool Input::handleMouseEvent(const SEvent::SMouseInput& ev)
{
	return false;
}
