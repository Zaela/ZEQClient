
#ifndef _ZEQ_WORLD_CONNECTION_H
#define _ZEQ_WORLD_CONNECTION_H

#include <string>
#include <vector>
#include "types.h"
#include "util.h"
#include "packet.h"
#include "connection.h"
#include "exception.h"
#include "login_connection.h"
#include "structs_titanium.h"
#include "renderer.h"

class WorldConnection : public Connection
{
private:
	CharacterSelect_Struct mCharacters;
	GuildsList_Struct* mGuildList;

	std::string mCharacterName;
	std::string mServerShortname;
	uint32 mExpansionInfo;
	std::string mMOTD;
	std::string mChatServer;
	std::string mChatServer2;
	ZoneServerInfo_Struct mZoneServer;

public:
	WorldConnection(LoginConnection* login);

	void process();
	bool processPacketQueue();
	bool processPacket(uint16 opcode, byte* data, uint32 len);
	void connect();
	bool hasCharacter(std::string name);
	bool zoneInCharacter(bool tutorial = false, bool gohome = false);
	void quickZoneInCharacter(std::string name);

	std::string getCharacterName() { return mCharacterName; }
	void setCharacterName(std::string name) { mCharacterName = name; }
	ZoneServerInfo_Struct* getZoneServer() { return &mZoneServer; }
	GuildsList_Struct* takeGuildList() { 
		GuildsList_Struct* ret = mGuildList;
		mGuildList = nullptr;
		return ret;
	}
};

#endif
