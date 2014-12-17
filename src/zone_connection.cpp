
#include "zone_connection.h"

ZoneConnection::ZoneConnection(WorldConnection* world) :
	Connection(world->getZoneServer()->ip, world->getZoneServer()->port),
	mCharacterName(world->getCharacterName()),
	mGuildList(world->takeGuildList())
{

}

void ZoneConnection::processInboundPackets()
{
	for (;;)
	{
		int len = recvWithTimeout(3000);
		if (!mPacketReceiver->handleProtocol(len))
			continue;
		//else we have some packets to process here
		std::queue<ReadPacket*>& queue = mAckMgr->getPacketQueue();
		while (!queue.empty())
		{
			ReadPacket* packet = queue.front();
			queue.pop();
			uint16 opcode = *(uint16*)packet->data;
			bool ret = processPacket(opcode, packet->data + 2, packet->len - 2);
			delete packet;
			if (ret)
				return;
		}
	}
}

bool ZoneConnection::processPacket(uint16 opcode, byte* data, uint32 len)
{
	switch (opcode)
	{
	case OP_PlayerProfile:
	{
		printf("OP_PlayerProfile\n");
		PlayerProfile_Struct* pp = (PlayerProfile_Struct*)data;
		break;
	}
	case OP_ZoneEntry:
	{
		printf("OP_ZoneEntry\n");
		Spawn_Struct* spawn = (Spawn_Struct*)data;
		break;
	}
	case OP_TimeOfDay:
	{
		printf("OP_TimeOfDay\n");
		TimeOfDay_Struct* td = (TimeOfDay_Struct*)data;
		printf("Time: %u:%u, %u/%u/%u\n", td->hour, td->minute, td->day, td->month, td->year);
		break;
	}
	case OP_CharInventory:
	{
		printf("OP_CharInventory\n");
		//block of serialized items

		break;
	}
	case OP_Weather:
	{
		printf("OP_Weather\n");
		Weather_Struct* w = (Weather_Struct*)data;

		//send OP_ReqNewZone here, server expects it shortly after this
		//and this is the best place to send from
		Packet packet(0, OP_ReqNewZone, mAckMgr);
		packet.send(this, getCRCKey());
		break;
	}
	case OP_NewZone:
	{
		printf("OP_NewZone\n");
		NewZone_Struct* nz = (NewZone_Struct*)data;
		printf("Zone: %s - %s\n", nz->zone_short_name, nz->zone_long_name);

		//send client spawn request
		Packet packet(0, OP_ReqClientSpawn, mAckMgr);
		packet.send(this, getCRCKey());
		break;
	}
	case OP_SendZonePoints:
	{
		printf("OP_SendZonePoints\n");
		uint32 count = *(uint32*)data;
		ZonePoints* zp = (ZonePoints*)(data + sizeof(uint32));
		break;
	}
	case OP_ZoneSpawns:
	{
		printf("OP_ZoneSpawns\n");
		//bulk spawn packet on zone-in only
		Spawn_Struct* spawn = (Spawn_Struct*)data;

		uint32 count = len / sizeof(Spawn_Struct);
		for (uint32 i = 0; i < count; ++i)
		{
			//handle spawn
		}
		break;
	}
	case OP_NewSpawn:
	{
		printf("OP_NewSpawn\n");
		//happens when a mob spawns!
		Spawn_Struct* spawn = (Spawn_Struct*)data;
		break;
	}
	case OP_SendAAStats:
	{
		printf("OP_SendAAStats\n");
		//the server sends us a 0 length one of these on zone in because live is weird
		if (len == 0)
			break;
		break;
	}
	case OP_SendExpZonein:
	{
		printf("OP_SendExpZonein\n");
		//the server sends us a 0 length one of these to tell us to send OP_ClientReady (for Titanium, anyway)
		if (len == 0)
		{
			//send client ready
			Packet packet(0, OP_ClientReady, mAckMgr);
			packet.send(this, getCRCKey());
			break;
		}
		break;
	}
	case OP_HPUpdate:
	{
		printf("OP_HPUpdate\n");
		//exact hp update
		ExactHPUpdate_Struct* hp = (ExactHPUpdate_Struct*)data;
		break;
	}
	case OP_MobHealth:
	{
		printf("OP_MobHealth\n");
		//percent hp update
		HPUpdate_Struct* hp = (HPUpdate_Struct*)data;
		break;
	}
	case OP_ManaChange:
	{
		printf("OP_ManaChange\n");
		ManaChange_Struct* mc = (ManaChange_Struct*)data;
		break;
	}
	case OP_SpawnAppearance:
	{
		printf("OP_SpawnAppearance\n");
		SpawnAppearance_Struct* ap = (SpawnAppearance_Struct*)data;
		break;
	}
	//packets we don't care about for now
	case OP_WearChange:
	case OP_SpawnDoor:
	case OP_GroundSpawn:
	case OP_SpecialMesg:
	case OP_TributeUpdate:
	case OP_TributeTimer:
	case OP_TaskDescription:
	case OP_TaskActivity:
	case OP_CompletedTasks:
		break;
	//packets we don't care about
	case OP_Unknown:
		break;
	default:
		printf("ZoneConnection received unknown opcode 0x%0.4X\n", opcode);
		break;
	}

	return false;
}

void ZoneConnection::connect()
{
	initiateConnection();

	Packet packet(sizeof(ClientZoneEntry_Struct), OP_ZoneEntry, mAckMgr);
	ClientZoneEntry_Struct* ze = (ClientZoneEntry_Struct*)packet.getDataBuffer();

	Util::strcpy(ze->char_name, mCharacterName.c_str(), 64);

	packet.send(this, getCRCKey());

	processInboundPackets();
}

void ZoneConnection::poll()
{
	for (;;)
	{
		int len = recvPacket();
		if (len <= 0)
			break;
		mPacketReceiver->handleProtocol(len);
	}

	std::queue<ReadPacket*>& queue = mAckMgr->getPacketQueue();
	while (!queue.empty())
	{
		ReadPacket* packet = queue.front();
		queue.pop();
		uint16 opcode = *(uint16*)packet->data;
		processPacket(opcode, packet->data + 2, packet->len - 2);
		delete packet;
	}
}
