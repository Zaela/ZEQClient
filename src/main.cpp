
#include <cstdarg>
#include <lua.hpp>

#include "socket.h"
#include "random.h"
#include "input.h"
#include "renderer.h"
#include "file_loader.h"
#include "player.h"
#include "login_connection.h"
#include "world_connection.h"
#include "zone_connection.h"
#include "zone_viewer.h"
#include "structs_eqg.h"
#include "mob_manager.h"
#include "zeq_lua.h"
#include "translate.h"

#include "s3d.h"
#include "wld.h"

Random gRNG;
Input gInput;
Renderer gRenderer;
FileLoader gFileLoader;
Player gPlayer;
MobManager gMobMgr;

void showError(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	char buf[1024];
	vsprintf(buf, fmt, va);

	printf("%s\n", buf);
#ifdef _WIN32
	MessageBox(NULL, buf, NULL, MB_OK | MB_ICONERROR | MB_TASKMODAL);
#endif
}

struct Args
{
	std::string acctName;
	std::string password;
	std::string serverName;
	std::string charName;
	std::string pathToEQ;
	std::string zoneShortname;
};

void readArgs(int c, char** args, Args& out);

int main(int argc, char** argv)
{
	LoginConnection* login = nullptr;
	WorldConnection* world = nullptr;
	ZoneConnection* zone = nullptr;

#ifdef _WIN32
	SetConsoleTitle("ZEQClient");
#endif

	try
	{
		Socket::loadLibrary();
		Lua::initialize();
		EQG_Structs::initialize();
		Translate::initialize();

		Lua::fileToTable(CONFIG_FILE, CONFIG_TABLE);

		Args args;
		readArgs(argc, argv, args);

		gRenderer.initialize();
		gFileLoader.setPathToEQ(args.pathToEQ);

		if (!args.zoneShortname.empty())
		{
			std::string shortname = args.zoneShortname;

			gPlayer.setZoneViewer(new ZoneViewerData);

			ZoneModel* zoneModel = ZoneModel::load(shortname);
			if (zoneModel == nullptr)
				throw ZEQException("bad zone shortname '%s'", shortname.c_str());

			gRenderer.useZoneModel(zoneModel);

			//WLD* wld = gFileLoader.getWLD("global_chr");
			//wld->convertMobModel("ELE");
			//wld->convertAllMobModels();
			gFileLoader.handleGlobalLoad();

			Mob* mob = gMobMgr.spawnMob(75, 2);
			gMobMgr.spawnMob(9, 0, 1, 5, 5, 5);
			gMobMgr.spawnMob(60, 2, 1, -5, 5, -5);
			//mob->startAnimation("T06");

			WLD* wld = gFileLoader.getWLD("dra_chr");
			wld->convertMobModel("DRA");

			gMobMgr.spawnMob(49, 2, 1, 20, 5, 20);

			gInput.setMode(Input::ZONE_VIEWER);
			gPlayer.setCamera(gRenderer.createCamera());
			gPlayer.zoneViewerLoop();
		}
		else
		{
			//do stuff
			login = new LoginConnection;
			login->setCredentials(args.acctName, args.password);
			login->quickConnect(args.serverName);

			gFileLoader.handleGlobalLoad();

			world = new WorldConnection(login);
			world->quickZoneInCharacter(args.charName);
			delete login;
			login = nullptr;

			zone = new ZoneConnection(world);
			gPlayer.setZoneConnection(zone);
			zone->connect();
			delete world;
			world = nullptr;

			gInput.setMode(Input::ZONE_VIEWER);
			gPlayer.setCamera(gRenderer.createCamera());
			gPlayer.mainLoop();
		}
	}
	catch (ZEQException& e)
	{
		showError("Exception: %s", e.what());
	}
	catch (ZEQBasicException& e)
	{
		showError("Basic exception: %s", e.getTypeName());
	}
	catch (std::exception& e)
	{
		showError("Uncaught standard exception: %s", e.what());
	}
	catch (ExitException)
	{
		printf("Exiting\n");
	}

	gRenderer.close();
	if (login) delete login;
	if (world) delete world;
	if (zone) delete zone;

	Lua::close();
	Socket::closeLibrary();
	return 0;
}

void printUsage();

void readArgs(int c, char** args, Args& out)
{
	//check config file first
	out.pathToEQ = Lua::getConfigString(CONFIG_VAR_EQ_PATH, "");
	out.acctName = Lua::getConfigString(CONFIG_VAR_ACCOUNT, "");
	out.charName = Lua::getConfigString(CONFIG_VAR_CHARACTER, "");
	out.serverName = Lua::getConfigString(CONFIG_VAR_SERVER, "");

	//read command line arguments
	int i = 1;
	while (i < c)
	{
		if (args[i][0] != '-')
			break;
		if (i + 1 == c)
			break;
		switch (args[i][1])
		{
		case 'a':
			out.acctName = args[i + 1];
			break;
		case 'p':
			out.password = args[i + 1];
			break;
		case 'c':
			out.charName = args[i + 1];
			break;
		case 's':
			out.serverName = args[i + 1];
			break;
		case 'e':
			out.pathToEQ = args[i + 1];
			break;
		case 'z':
			out.zoneShortname = args[i + 1];
			break;
		default:
			goto FINISH;
		}
		i += 2;
	}
FINISH:
	if (out.pathToEQ.size() && out.zoneShortname.size())
		return;
	if (!out.pathToEQ.size() || !out.acctName.size() || !out.password.size() || !out.charName.size() || !out.serverName.size())
	{
		printUsage();
		throw ExitException();
	}
}

void printUsage()
{
	printf("Usage:\n"
		"To connect to a server:\n"
		"\t-e <path\\to\\eq>\n"
		"\t-a <account name>\n"
		"\t-p <password>\n"
		"\t-c <character name>\n"
		"\t-s <server longname>\n"
		"To launch the zone viewer:\n"
		"\t-e <path\\to\\eq>\n"
		"\t-z <zone shortname>\n");
}
