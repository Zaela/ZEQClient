
#ifndef _ZEQ_LUA_H
#define _ZEQ_LUA_H

#include <lua.hpp>
#include <string>
#include "types.h"

#define CONFIG_TABLE "config"
#define CONFIG_FILE "config.lua"

#define CONFIG_VAR_EQ_PATH "EQPath"
#define CONFIG_VAR_ACCOUNT "Account"
#define CONFIG_VAR_CHARACTER "Character"
#define CONFIG_VAR_SERVER "Server"
#define CONFIG_VAR_LOGIN_IP "LoginIP"
#define CONFIG_VAR_LOGIN_PORT "LoginPort"
#define CONFIG_VAR_SCREEN_WIDTH "ScreenWidth"
#define CONFIG_VAR_SCREEN_HEIGHT "ScreenHeight"
#define CONFIG_VAR_VSYNC "Vsync"
#define CONFIG_VAR_FULLSCREEN "Fullscreen"
#define CONFIG_VAR_RENDERER "Renderer"
#define CONFIG_VAR_SHOW_ZONE_WALLS "ShowZoneWalls"

namespace Lua
{
	void initialize();
	void close();
	void fileToTable(const char* filePath, const char* table);

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
}

#endif
