
#include "packet_receiver.h"

PacketReceiver::PacketReceiver(Socket* socket, AckManager* ackMgr, bool isLogin)
	: mSocket(socket), mAckMgr(ackMgr), mIsLogin(isLogin)
{
	mIsDisconnected = false;
}

bool PacketReceiver::handleProtocol(uint32 len)
{
	readPacket(mSocket->getBuffer(), len);
	return mAckMgr->hasQueuedPackets();
}

void PacketReceiver::readPacket(byte* data, uint32 len, bool fromCombined)
{
	uint16 opcode = toHostShort(*(uint16*)data);
	uint32 offset = 2;
	if (opcode > 0xFF)
	{
		//raw packet, no protocol
		mAckMgr->queueRawPacket(data, len);
		return;
	}

	switch (opcode)
	{
	case OP_SessionResponse:
	{
		printf("OP_SessionResponse\n");
		SessionResponse* sr = (SessionResponse*)(data + 2);
		mCRCKey = toHostLong(sr->key);

		mAckMgr->setCRCKey(mCRCKey);

		if (mIsLogin)
		{
			//send session ready packet
			Packet packet(12, OP_SessionReady, mAckMgr, OP_Packet, false, false);
			byte* b = packet.getDataBuffer();
			b[0] = 2;
			b[10] = 8;

			packet.send(mSocket, mCRCKey);
		}
		else
		{
			//echo session response to the server
			mSocket->sendPacket(data, len);
		}

		printf("server CRC key: %i (0x%0.8X)\n", mCRCKey, mCRCKey);
		break;
	}
	case OP_Combined:
	{

		offset = validateCompletePacket(data, len, fromCombined);
		if (!fromCombined && offset == 0xFF)
			break;
		uint32 pos = offset;
		while (pos < len)
		{
			//8 bit packet length
			uint8 size = data[pos];
			++pos;

			uint32 avail = len - pos;
			if (size > avail)
				size = avail;

			readPacket(data + pos, size, true);
			pos += size;
		}
		break;
	}
	case OP_Packet:
	{

		offset = validateCompletePacket(data, len, fromCombined);
		if (!fromCombined && offset == 0xFF)
			break;

		mAckMgr->checkInboundPacket(data, len, offset);
		break;
	}
	case OP_Fragment:
	{
		offset = validateCompletePacket(data, len, fromCombined);
		if (!fromCombined && offset == 0xFF)
			break;

		mAckMgr->checkInboundFragment(data, len);
		break;
	}
	case OP_Ack:
	{
		offset = validateCompletePacket(data, len, fromCombined);
		if (!fromCombined && offset == 0xFF)
			break;
		if(offset == 0xFF)
			break;

		uint16 seq = toHostShort(*(uint16*)(data + offset));
		mAckMgr->receiveAck(seq);
		break;
	}
	case OP_SessionDisconnect:
		SetDisconnected(true);
		break;
	//packets we don't care about
	case OP_SessionStatResponse:
		break;
	default:
		printf("PacketReceiver received unknown protocol opcode 0x%0.4X len: %u, fromCombined: %i\n", opcode,
			len, fromCombined ? 1 : 0);
		for (uint32 i = 0; i < len; ++i)
			printf("%0.2X ", data[i]);
		printf("\n");
		break;
	}
}

//signed int is offset for packet - 0xFF = INVALID
signed int PacketReceiver::validateCompletePacket(byte*& packet, uint32& len, bool fromCombined)
{
	//check CRC before decompressing
	if(!fromCombined)
	{
		if (!NetworkCRC::validatePacket(packet, len, mCRCKey))
			return 0xFF;
	}
	//attempt to decompress
	//if not unencrypted flag
	if(packet[2] == 0x5a) //compressed
	{
		if (!Compression::decompressPacket(packet, len))
			return 2;
	}
	else if(packet[2] == 0xa5) //Not compressed, single byte flag
	{
		return 3;
	}

	return 2;
}
