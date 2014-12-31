
#ifndef _ZEQ_PACKET_H
#define _ZEQ_PACKET_H

#include "types.h"
#include "opcodes.h"
#include "socket.h"
#include "packet_protocol.h"
#include "network_crc.h"
#include "compression.h"

class AckManager;

class Packet
{
private:
	uint16 mLen;
	uint16 mDataLen;
	uint8 mDataPos;
	bool mHasCRC;
	bool mCompress;
	byte* mBuffer;

private:
	void writeCRC(uint32 crcKey);
	void compress();

public:
	Packet(int data_len, uint16 opcode, AckManager* ackMgr, int protocol_opcode = OP_Packet,
		bool no_crc = false, bool compressed = true);
	Packet();
	Packet(const Packet& toCopy);
	~Packet();

	uint16 length() { return mDataLen; }
	uint16 lengthWithOverhead() { return mLen; }
	byte* getDataBuffer() { return mBuffer + mDataPos; }
	void send(Socket* socket, uint32 crcKey);
	void setSequence(uint16 seq) { *(uint16*)(mBuffer + 2) = toNetworkShort(seq); }
};

struct ReadPacket
{
	ReadPacket() : data(nullptr), len(0) { }

	ReadPacket(byte* in_data, uint32 in_len) : len(in_len)
	{
		data = new byte[in_len];
		if (in_data)
			memcpy(data, in_data, in_len);
	}

	~ReadPacket()
	{
		if (data)
			delete[] data;
	}

	byte* data;
	uint32 len;
	uint32 max_seq;
};

#endif
