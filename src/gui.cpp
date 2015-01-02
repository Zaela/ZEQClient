
#include "gui.h"

extern GUI gGUI;

using namespace Rocket::Core;

uint32 GUI::getColorForChannel(int channel)
{
	if (isColorChannel(channel))
	{
		switch (channel)
		{
		case 12: //yellow
			return 0xFFFF00;
		case 10: //white
		default:
			return 0xFFFFFF;
		}
	}

	if (mChatColorsByChannel.count((ChatChannel)channel))
		return mChatColorsByChannel[(ChatChannel)channel];

	return 0xFFFFFF; //white
}

void GUI::displayChat(int channel, String str)
{
	uint32 color = getColorForChannel(channel);
	char hexclr[7];
	sprintf(hexclr, "%0.6X", color);

	ChatWindow* wnd = nullptr;
	if (mChatWindowsByChannel.count(CHANNEL_ALL))
		wnd = &mChatWindowsByChannel[CHANNEL_ALL];
	else if (isColorChannel(channel))
		wnd = &mChatWindowsByChannel[CHANNEL_COLOR];
	else if (mChatWindowsByChannel.count((ChatChannel)channel))
		wnd = &mChatWindowsByChannel[(ChatChannel)channel];

	if (wnd == nullptr)
		return;

	String rml;
	wnd->elem->GetInnerRML(rml);
	wnd->lines++;

	if (color != 0xFFFFFF)
	{
		rml += "<span style=\"color:#";
		rml += hexclr;
		rml += ";\">";
		rml += str;
		rml += "</span><br/>";
	}
	else
	{
		rml += str;
		rml += "<br/>";
	}

	wnd->elem->SetInnerRML(rml);
	wnd->elem->SetScrollTop(9999999.0f);
}

void GUI::registerChatWindow(ChatChannel channel, Element* elem)
{
	ChatWindow wnd;
	wnd.lines = 0;
	wnd.elem = elem;

	mChatWindowsByChannel[channel] = wnd;
}

int GUI::registerChatWindow(lua_State* L)
{
	Element** elem = (Element**)lua_touserdata(L, 1);
	int channel;

	if (lua_isnil(L, 2))
		channel = CHANNEL_ALL;
	else
		channel = lua_tointeger(L, 2);

	gGUI.registerChatWindow((ChatChannel)channel, *elem);
	return 0;
}

void GUI::loadLuaFunctions(lua_State* L)
{
	lua_newtable(L);

	auto lua_enum = [=](const char* name, int val)
	{
		lua_pushinteger(L, val);
		lua_setfield(L, -2, name);
	};

	//ChatChannels enum
	lua_newtable(L);
	lua_enum("CHANNEL_COLOR", CHANNEL_COLOR);
	lua_enum("CHANNEL_ALL", CHANNEL_ALL);
	lua_setfield(L, -2, "chat_channels");

	luaL_Reg funcs[] = {
		{"registerChatWindow", registerChatWindow},
		{nullptr, nullptr}
	};

	luaL_register(L, nullptr, funcs);
	lua_setglobal(L, "gGUI");
}
