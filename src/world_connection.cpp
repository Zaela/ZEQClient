
#include "world_connection.h"
#include "gui.h"

extern Renderer gRenderer;
extern GUI gGUI;

WorldConnection::WorldConnection(LoginConnection* login) :
	Connection(login->getServer()->ip.c_str(), 9000),
	mGuildList(nullptr)
{
	inheritSession(login);
}

void WorldConnection::process()
{
	connect();
	while(g_EqState == World)
	{
		bool Disconnected = processPacketQueue();
		if(Disconnected)
		{
			g_EqState = Login;
			return;
		}
	}
}


bool WorldConnection::processPacketQueue()
{
	if(mPacketReceiver->GetDisconnected())
		return true;

	int len = recvWithTimeout(5000);

	if(len == 0)
	{
		mPacketReceiver->SetDisconnected(true);
		return true;
	}

	if (!mPacketReceiver->handleProtocol(len))
	{
		return false;
	}

	//else we have some packets to process here
	std::queue<ReadPacket*>& queue = mAckMgr->getPacketQueue();
	while (!queue.empty())
	{
		ReadPacket* packet = queue.front();
		queue.pop();
		uint16 opcode = *(uint16*)packet->data;
		bool ret = processPacket(opcode, packet->data + 2, packet->len - 2);
		if(!ret)
		{
			printf("Invalid packet in world state.\n");
		}
		delete packet;
	}
	return false;
}

bool WorldConnection::processPacket(uint16 opcode, byte* data, uint32 len)
{
	switch (opcode)
	{
	case OP_GuildsList:
	{
		//big dumb packet
		//regardless of actual number of guilds, there are 1501 64 byte spaces for their names
		//this is the only time we get them all though, so we need to hang on to them
		if (mGuildList)
			delete mGuildList;

		GuildsList_Struct* guilds = (GuildsList_Struct*)data;
		mGuildList = new GuildsList_Struct;

		for (int i = 0; i < MAX_NUMBER_GUILDS; ++i)
		{
			if (guilds->Guilds[i].name[0] != 0)
				Util::strcpy(mGuildList->Guilds[i].name, guilds->Guilds[i].name, 64);
			else
				mGuildList->Guilds[i].name[0] = 0;
		}
		break;
	}
	case OP_LogServer:
	{
		//we care about exactly one part of this packet
		mServerShortname = std::string((char*)(data + 32));
		break;
	}
	case OP_SendCharInfo:
	{
		//this is where we get all the characters we have to choose from at char select
		memcpy(&mCharacters, data, sizeof(CharacterSelect_Struct));

		printf("Characters available:\n");
		for (int i = 0; i < 10; ++i)
		{
			if (mCharacters.level[i] != 0)
				printf("%s\n", mCharacters.name[i]);
		}

		bool returnVal = zoneInCharacter();
		if(!returnVal)
		{
			printf("Character is not available:\n");
			g_EqState = Login;
			return returnVal;
		}
		return true;
	}
	case OP_ExpansionInfo:
	{
		//just a uint32
		mExpansionInfo = *(uint32*)data;
		break;
	}
	case OP_MOTD:
	{
		gRenderer.loadGUI(Renderer::GUI_ZONE);

		mMOTD = std::string((char*)data, len);
		printf("Received MOTD: %s\n", mMOTD.c_str());
		gGUI.displayChat(12, mMOTD.c_str());
		//GUI::addChat(0, mMOTD.c_str());
		break;
	}
	case OP_SetChatServer:
	{
		mChatServer = std::string((char*)data, len);
		printf("ChatServer: %s\n", mChatServer.c_str());
		break;
	}
	case OP_SetChatServer2:
	{
		mChatServer2 = std::string((char*)data, len);
		printf("ChatServer2: %s\n", mChatServer2.c_str());
		break;
	}
	case OP_ZoneUnavail:
	{
		throw ZEQException("Zone unavailable");
	}
	case OP_ZoneServerInfo:
	{
		memcpy(&mZoneServer, data, sizeof(ZoneServerInfo_Struct));

		printf("Received ZoneServerInfo: %s : %u\n", mZoneServer.ip, mZoneServer.port);
		g_EqState = Zone;
		break;
	}
	//packets we don't care about
	case OP_ApproveWorld: //nothing meaningful
	case OP_EnterWorld: //empty packet
	case OP_PostEnterWorld: //empty packet - we're probably supposed to trigger state changes of some kind
		break;
	default:
		printf("WorldConnection received unknown opcode 0x%0.4X\n", opcode);
		return false;
	}

	return true;
}

void WorldConnection::connect()
{
	initiateConnection();

	Packet packet(sizeof(LoginInfo_Struct), OP_SendLoginInfo, mAckMgr);
	LoginInfo_Struct* li = (LoginInfo_Struct*)packet.getDataBuffer();

	//login_info -> accountID as a string, null terminator, session key
	memset(li->login_info, 0, 64);
	itoa(getAccountID(), li->login_info, 10);
	memcpy(&li->login_info[strlen(li->login_info) + 1], getSessionKey().c_str(), getSessionKey().length());

	packet.send(this, getCRCKey());
}

bool WorldConnection::hasCharacter(std::string name)
{
	for (int i = 0; i < 10; ++i)
	{
		if (name.compare(mCharacters.name[i]) == 0)
			return true;
	}
	return false;
}

bool WorldConnection::zoneInCharacter(bool tutorial, bool gohome)
{
	if (!hasCharacter(mCharacterName))
		return false;

	Packet packet(sizeof(EnterWorld_Struct), OP_EnterWorld, mAckMgr);
	EnterWorld_Struct* ew = (EnterWorld_Struct*)packet.getDataBuffer();

	Util::strcpy(ew->name, mCharacterName.c_str(), 64);

	if (tutorial)
		ew->tutorial = 1;
	else if (gohome)
		ew->return_home = 1;

	packet.send(this, getCRCKey());
	return true;
}

void WorldConnection::quickZoneInCharacter(std::string name)
{
	this->setCharacterName(name);
}
