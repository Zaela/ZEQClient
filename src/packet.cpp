
#include "packet.h"
#include "ack_manager.h"

Packet::Packet(int data_len, uint16 opcode, AckManager* ackMgr, int protocol_opcode, bool no_crc, bool compressed) :
	mBuffer(nullptr)
{
	uint16 len = data_len + 8;
	uint8 dataPos = 6;
	bool hasCRC = true;
	if (ackMgr == nullptr)
	{
		len -= 2;
		dataPos = 4;
	}
	if (no_crc)
	{
		len -= 2;
		hasCRC = false;
	}

	byte* buf = new byte[len];
	memset(buf, 0, len);

	mLen = len;
	mDataLen = data_len;
	mDataPos = dataPos;
	mHasCRC = hasCRC;
	mCompress = compressed;
	mBuffer = buf;

	uint16* ptr = (uint16*)buf;
	*ptr = toNetworkShort(protocol_opcode);

	if (opcode != OP_NONE)
	{
		ptr[ackMgr ? 2 : 1] = opcode;
	}
	else
	{
		mLen -= 2;
		mDataPos -= 2;
	}

	if (ackMgr)
	{
		uint16 seq = toNetworkShort(ackMgr->getNextSequence());
		ptr[1] = seq;
		ackMgr->recordSentPacket(*this, seq);
	}
}

Packet::Packet() :
	mLen(0),
	mDataLen(0),
	mDataPos(0),
	mHasCRC(false),
	mBuffer(nullptr)
{

}

Packet::Packet(const Packet& toCopy) :
	mLen(toCopy.mLen),
	mDataLen(toCopy.mDataLen),
	mDataPos(toCopy.mDataPos),
	mHasCRC(toCopy.mHasCRC)
{
	mBuffer = new byte[mLen];
	memcpy(mBuffer, toCopy.mBuffer, mLen);
}

Packet::~Packet()
{
	if (mBuffer)
		delete[] mBuffer;
}

void Packet::writeCRC(uint32 crcKey)
{
	if (mBuffer == nullptr)
		return;
	uint16 len = mLen - 2;
	uint16* ptr = (uint16*)(mBuffer + len);
	*ptr = NetworkCRC::calcOutbound(mBuffer, len, crcKey);
}

void Packet::send(Socket* socket, uint32 crcKey)
{
	if (mCompress)
		compress();
	if (mHasCRC)
		writeCRC(crcKey);
	socket->sendPacket(mBuffer, mLen);
}

void Packet::compress()
{
	//compress everything except the protocol opcode and crc space
	//I think every compressed packet also has a crc...
	byte* data = mBuffer + 2;
	uint32 len = mLen - 4;
	Compression::compressBlock(data, len);
	//check results - we add 1 byte for the compression flag
	if (len > (uint32)(mLen - 5))
	{
		//compression increased our size, need to realloc buffer
		uint16 opcode = *(uint16*)mBuffer;
		delete[] mBuffer;
		mBuffer = new byte[len + 5];
		*(uint16*)mBuffer = opcode;
	}

	mBuffer[2] = 'Z';
	memcpy(&mBuffer[3], data, len);
	mLen = len + 5;
}
