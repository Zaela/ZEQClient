
#include <cstdarg>
#include <lua.hpp>

#include "socket.h"
#include "eq_state.h"
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
#include "eqstr.h"
#include "rocket.h"
#include "gui.h"

#include "s3d.h"
#include "wld.h"

Random gRNG;
Input gInput;
Renderer gRenderer;
FileLoader gFileLoader;
Player gPlayer;
MobManager gMobMgr;
GUI gGUI;
EqState g_EqState;

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

		Args args;
		readArgs(argc, argv, args);

		gRenderer.initializeGUI();
		gRenderer.initialize();
		gFileLoader.setPathToEQ(args.pathToEQ);

		if (!args.zoneShortname.empty())
		{
			std::string shortname = args.zoneShortname;

			gRenderer.loadGUI(Renderer::GUI_VIEWER);
			gPlayer.setZoneViewer(new ZoneViewerData);

			ZoneModel* zoneModel = ZoneModel::load(shortname);
			if (zoneModel == nullptr)
				throw ZEQException("bad zone shortname '%s'", shortname.c_str());

			gRenderer.useZoneModel(zoneModel);

			gInput.setMode(Input::ZONE_VIEWER);
			gPlayer.setCamera(gRenderer.createCamera());
			gPlayer.zoneViewerLoop();
		}
		else
		{
			//do stuff
			g_EqState = Login;
			while(g_EqState != None)
			{
				EQStr::initialize(gFileLoader.getPathToEQ());
				
				if(g_EqState == Login)
				{
					if(login)
					{
						delete login;
						login = nullptr;
					}
					//clean up previous world instance since we're starting over
					if(world)
					{
						delete world;
						world= nullptr;
					}
					//clean up previous zone instance since we're starting over
					if(zone)
					{
						delete zone;
						zone= nullptr;
					}

					login = new LoginConnection;
					login->setCredentials(args.acctName, args.password);
					login->setServerName(args.serverName);
					login->process();
				}
				else if(g_EqState == World) 
				{
					if(login)
					{
						gFileLoader.handleGlobalLoad();

						if(!world)
						{
							world = new WorldConnection(login);
						}
						world->quickZoneInCharacter(args.charName);
						world->process();
					}
					else
					{
						g_EqState = Login;
					}
				}
				else if(g_EqState == Zone)
				{
					if(zone)
					{
						delete zone;
						zone = nullptr;
					}
					if(world)
					{
						zone = new ZoneConnection(world);
						gPlayer.setZoneConnection(zone);
						zone->connect();
						gInput.setMode(Input::ZONE);
						gPlayer.setCamera(gRenderer.createCamera());
						gPlayer.mainLoop();
					}
					else
					{
						g_EqState = Login;
					}
				}
			}
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
