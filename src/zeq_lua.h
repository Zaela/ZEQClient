
#ifndef _ZEQ_LUA_H
#define _ZEQ_LUA_H

#include <lua.hpp>
#include <string>
#include <unordered_map>
#include "rocket.h"
#include "types.h"

#define CONFIG_TABLE "config"
#define CONFIG_FILE "config.lua"

#define CONFIG_VAR_EQ_PATH "eqpath"
#define CONFIG_VAR_ACCOUNT "account"
#define CONFIG_VAR_CHARACTER "character"
#define CONFIG_VAR_SERVER "server"
#define CONFIG_VAR_LOGIN_IP "loginip"
#define CONFIG_VAR_LOGIN_PORT "loginport"
#define CONFIG_VAR_SCREEN_WIDTH "screenwidth"
#define CONFIG_VAR_SCREEN_HEIGHT "screenheight"
#define CONFIG_VAR_VSYNC "vsync"
#define CONFIG_VAR_FULLSCREEN "fullscreen"
#define CONFIG_VAR_RENDERER "renderer"
#define CONFIG_VAR_SHOW_ZONE_WALLS "showzonewalls"

namespace Lua
{
	void initialize();
	void close();
	lua_State* getState();
	void fileToTable(const char* filePath, const char* table, bool lowercase_keys = true);
	void fileToHashTable(const char* filePath, std::unordered_map<std::string, int, std::hash<std::string>>& table);

	int getInt(const char* varname, const char* table);
	int getInt(const char* varname, const char* table, int default);

	std::string getString(const char* varname, const char* table);
	std::string getString(const char* varname, const char* table, const char* default);

	bool getBool(const char* varname, const char* table);
	bool getBool(const char* varname, const char* table, bool default);

	int getConfigInt(const char* varname);
	int getConfigInt(const char* varname, int default);
	std::string getConfigString(const char* varname);
	std::string getConfigString(const char* varname, const char* default);
	bool getConfigBool(const char* varname);
	bool getConfigBool(const char* varname, bool default);

	void loadFontsGUI(); //do this on the lua side instead
	Rocket::Core::Context* initGUI(int width, int height);
	Rocket::Core::ElementDocument* initGUIDocument(const char* path);
}

namespace GUI
{
	void addChat(int channel, const char* message);
}

#endif
