
#ifndef _ZEQ_GUI_H
#define _ZEQ_GUI_H

#include <unordered_map>
#include <string>

#include "types.h"
#include "rocket.h"
#include "zeq_lua.h"

class GUI
{
private:
	struct ChatWindow
	{
		Rocket::Core::Element* elem;
		int lines;
	};

	enum ChatChannel
	{
		CHANNEL_COLOR = 0,
		CHANNEL_ALL
	};

	std::unordered_map<ChatChannel, ChatWindow> mChatWindowsByChannel;
	std::unordered_map<ChatChannel, uint32> mChatColorsByChannel;

private:
	static bool isColorChannel(int chan) { return chan < 100; }
	uint32 getColorForChannel(int channel);

public:
	void initialize();

	void displayChat(int channel, Rocket::Core::String str);

	void registerChatWindow(ChatChannel channel, Rocket::Core::Element* window);

private:
	//lua functions
	static int registerChatWindow(lua_State* L);

public:
	void loadLuaFunctions(lua_State* L);
};

#endif
