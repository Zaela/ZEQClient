
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
		//printf("OP_Combined\n");
		if (!fromCombined && !validateCompletePacket(data, len))
			break;

		uint32 pos = 2;
		while (pos < len)
		{
			//8 bit packet length
			uint8 size = data[pos];
			++pos;

			readPacket(data + pos, size, true);
			pos += size;
		}
		break;
	}
	case OP_Packet:
	{
		//printf("OP_Packet\n");
		if (!fromCombined && !validateCompletePacket(data, len))
			break;

		mAckMgr->checkInboundPacket(data, len);
		break;
	}
	case OP_Fragment:
	{
		//printf("OP_Fragment\n");
		if (!fromCombined && !validateCompletePacket(data, len))
			break;

		mAckMgr->checkInboundFragment(data, len);
		break;
	}
	case OP_Ack:
	{
		//printf("OP_Ack\n");
		if (!fromCombined && !validateCompletePacket(data, len))
			break;

		uint16 seq = toHostShort(*(uint16*)(data + 2));
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

bool PacketReceiver::validateCompletePacket(byte*& packet, uint32& len)
{
	//check CRC before decompressing
	if (!NetworkCRC::validatePacket(packet, len, mCRCKey))
		return false;
	//attempt to decompress
	len -= 2; //crc
	if (!Compression::decompressPacket(packet, len))
		return false;
	return true;
}
