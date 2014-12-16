
#include "packet_receiver.h"

PacketReceiver::PacketReceiver(Socket* socket, AckManager* ackMgr, bool isLogin)
	: mSocket(socket), mAckMgr(ackMgr), mIsLogin(isLogin)
{

}

bool PacketReceiver::handleProtocol(uint32 len)
{
	readPacket(mSocket->getBuffer(), len);
	return mAckMgr->hasQueuedPackets();
}

void PacketReceiver::readPacket(byte* data, uint32 len, bool fromCombined)
{
	uint16 opcode = toHostShort(*(uint16*)data);

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
		printf("OP_Combined\n");
		if (!validateCompletePacket(data, len))
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
		printf("OP_Packet\n");
		if (!fromCombined && !validateCompletePacket(data, len))
			break;

		mAckMgr->checkInboundPacket(data, len);
		break;
	}
	case OP_Fragment:
	{
		printf("OP_Fragment\n");
		if (!fromCombined && !Compression::decompressPacket(data, len))
			break;

		mAckMgr->checkInboundFragment(data, len);
		break;
	}
	case OP_Ack:
	{
		printf("OP_Ack\n");
		if (!fromCombined && !validateCompletePacket(data, len))
			break;

		uint16 seq = toHostShort(*(uint16*)(data + 2));
		mAckMgr->receiveAck(seq);
		break;
	}
	case OP_SessionDisconnect:
		throw DisconnectException();
	//packets we don't care about
	case OP_SessionStatResponse:
		break;
	default:
		printf("PacketReceiver received unknown protocol opcode 0x%0.4X\n", opcode);
		break;
	}
}

bool PacketReceiver::validateCompletePacket(byte*& packet, uint32& len)
{
	//check CRC before decompressing
	if (!NetworkCRC::validatePacket(packet, len, mCRCKey))
		return false;
	//attempt to decompress
	if (!Compression::decompressPacket(packet, len))
		return false;
	return true;
}
