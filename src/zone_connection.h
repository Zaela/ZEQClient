
#ifndef _ZEQ_ZONE_CONNECTION_H
#define _ZEQ_ZONE_CONNECTION_H

#include "types.h"
#include "util.h"
#include "packet.h"
#include "connection.h"
#include "exception.h"
#include "world_connection.h"
#include "structs_titanium.h"

class ZoneConnection : public Connection
{
private:
	std::string mCharacterName;
	GuildsList_Struct* mGuildList;

public:
	ZoneConnection(WorldConnection* world);

	void processInboundPackets();
	bool processPacket(uint16 opcode, byte* data, uint32 len);
	void connect();
};

#endif
