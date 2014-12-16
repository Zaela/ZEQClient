
#ifndef _ZEQ_NETWORK_CRC_H
#define _ZEQ_NETWORK_CRC_H

#include "types.h"
#include "socket.h"

namespace NetworkCRC
{
	uint16 calc(void* data, int len, uint32 key);
	uint16 calcOutbound(void* data, int len, uint32 key);
	bool validatePacket(void* packet, int len, uint32 key);
}

#endif
