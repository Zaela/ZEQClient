
#ifndef _ZEQ_PACKET_RECEIVER_H
#define _ZEQ_PACKET_RECEIVER_H

#include <queue>

#include "types.h"
#include "socket.h"
#include "ack_manager.h"
#include "packet.h"
#include "packet_protocol.h"
#include "compression.h"
#include "exception.h"

class PacketReceiver
{
private:
	Socket* mSocket;
	AckManager* mAckMgr;
	uint32 mCRCKey;
	bool mIsLogin;
	bool mIsDisconnected;

private:
	bool validateCompletePacket(byte*& packet, uint32& len);
	void readPacket(byte* data, uint32 len, bool fromCombined = false);

public:
	PacketReceiver(Socket* socket, AckManager* ackMgr, bool isLogin = false);

	bool handleProtocol(uint32 len);
	bool IsLogin() { return mIsLogin; }
	void SetDisconnected(bool state) { mIsDisconnected = state; }
	bool GetDisconnected() { return mIsDisconnected; }

};

#endif
