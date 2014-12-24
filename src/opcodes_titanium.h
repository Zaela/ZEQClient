
#ifndef _ZEQ_OPCODES_TITANIUM_H
#define _ZEQ_OPCODES_TITANIUM_H

//world packets
#define OP_SendLoginInfo 0x4DD0
#define OP_GuildsList 0x6957
#define OP_LogServer 0x0FA6
#define OP_ApproveWorld 0x3C25
#define OP_SendCharInfo 0x4513
#define OP_EnterWorld 0x7CBA
#define OP_PostEnterWorld 0x52A4
#define OP_ExpansionInfo 0x04EC
#define OP_AckPacket 0x7752
#define OP_MOTD 0x024D
#define OP_SetChatServer 0x00D7
#define OP_SetChatServer2 0x6536
#define OP_ZoneUnavail 0x407C
#define OP_ZoneServerInfo 0x61B6

//zone-in packets
#define OP_ZoneEntry 0x7213
#define OP_PlayerProfile 0x75DF
#define OP_TimeOfDay 0x1580
#define OP_CharInventory 0x5394
#define OP_ReqNewZone 0x7AC5
#define OP_NewZone 0x0920
#define OP_ReqClientSpawn 0x0322
#define OP_SpawnDoor 0x4C24
#define OP_SendZonePoints 0x3EBA
#define OP_SendExpZonein 0x0587
#define OP_ClientReady 0x5E20
#define OP_ZoneSpawns 0x2E78

//mob-related packets
#define OP_NewSpawn 0x1860
#define OP_DeleteSpawn 0x55BC
#define OP_SpawnAppearance 0x7C32
#define OP_WearChange 0x7441
#define OP_HPUpdate 0x3BCF
#define OP_MobHealth 0x0695
#define OP_ManaChange 0x4839
#define OP_MobUpdate 0x14CB

//general zone packets
#define OP_Weather 0x254D
#define OP_GroundSpawn 0x0F47
#define OP_SpecialMesg 0x2372

//AA packets
#define OP_SendAAStats 0x5996

//tribute packets
#define OP_TributeUpdate 0x5639
#define OP_TributeTimer 0x4665

//task packets
#define OP_TaskDescription 0x5EF7
#define OP_TaskActivity 0x682D
#define OP_CompletedTasks 0x76A2

#endif
