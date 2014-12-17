
#include <cstdarg>

#include "socket.h"
#include "random.h"
#include "input.h"
#include "renderer.h"
#include "file_loader.h"
#include "player.h"
#include "login_connection.h"
#include "world_connection.h"
#include "zone_connection.h"

#include "s3d.h"
#include "wld.h"

Random gRNG;
Input gInput;
Renderer gRenderer;
FileLoader gFileLoader;
Player gPlayer;

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

		Args args;
		readArgs(argc, argv, args);

		gRenderer.initialize();
		gFileLoader.setPathToEQ(args.pathToEQ);

		if (!args.zoneShortname.empty())
		{
			std::string shortname = args.zoneShortname;

			WLD* wld = gFileLoader.getWLD(shortname);
			if (wld == nullptr)
				throw ZEQException("bad zone shortname");
			ZoneModel* zoneModel = wld->convertZoneGeometry();
			WLD* objWLD = gFileLoader.getWLD(shortname + "_obj");
			objWLD->convertZoneObjectDefinitions(zoneModel);
			WLD* placeWLD = gFileLoader.getWLD("objects", shortname.c_str());
			placeWLD->convertZoneObjectPlacements(zoneModel);

			gRenderer.useZoneModel(zoneModel);

			gPlayer.setCamera(gRenderer.createCamera());
			gPlayer.zoneViewerLoop();
		}
		else
		{
			//do stuff
			login = new LoginConnection;
			login->setCredentials(args.acctName, args.password);
			login->quickConnect(args.serverName);

			world = new WorldConnection(login);
			world->quickZoneInCharacter(args.charName);

			zone = new ZoneConnection(world);
			zone->connect();
		}
	}
	catch (ZEQException& e)
	{
		showError("Uncaught exception: %s", e.what());
	}
	catch (ZEQBasicException& e)
	{
		showError("Uncaught basic exception: %s", e.getTypeName());
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

	Socket::closeLibrary();
	return 0;
}

void printUsage();

void readArgs(int c, char** args, Args& out)
{
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
	if (!out.pathToEQ.empty() && !out.zoneShortname.empty())
		return;
	if (out.pathToEQ.empty() || out.acctName.empty() || out.password.empty() || out.charName.empty() || out.serverName.empty())
	{
		printUsage();
		throw ZEQException("bad arguments");
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
