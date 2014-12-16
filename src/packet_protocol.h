
#ifndef _ZEQ_PACKET_PROTOCOL_H
#define _ZEQ_PACKET_PROTOCOL_H

#include "types.h"

#pragma pack(1)

struct SessionRequest
{
	uint16 opcode;
	uint32 unknown;
	uint32 sessionID;
	uint32 maxLength;
};

struct SessionResponse
{
	uint32 session;
	uint32 key;
	uint8 unknownA;
	uint8 format;
	uint8 unknownB;
	uint32 maxLength;
	uint32 unknownC;
};

struct SessionStat
{
	uint16 requestID;
	uint32 last_local_delta;
	uint32 average_delta;
	uint32 low_delta;
	uint32 high_delta;
	uint32 last_remote_delta;
	uint64 packets_sent;
	uint64 packets_received;
};

#pragma pack()

#define OP_SessionRequest 0x01
#define OP_SessionReady 0x01
#define OP_SessionResponse 0x02
#define OP_Combined 0x03
#define OP_SessionDisconnect 0x05
#define OP_KeepAlive 0x06
#define OP_SessionStatRequest 0x07
#define OP_SessionStatResponse 0x08
#define OP_Packet 0x09
#define OP_Fragment 0x0D
#define OP_OutOfOrder 0x11
#define OP_Ack 0x15

#endif
