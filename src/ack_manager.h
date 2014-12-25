
#ifndef _ZEQ_ACK_MANAGER_H
#define _ZEQ_ACK_MANAGER_H

#include <queue>

#include "types.h"
#include "socket.h"
#include "packet.h"
#include "random.h"

class AckManager
{
private:
	static const uint32 SEQUENCE_MAX = 65536;
	static const uint16 WINDOW_SIZE = 2048;

	Socket* mSocket;
	uint32 mCRCKey;
	uint32 mSessionID;

	//these overflow by design
	uint16 mNextSeq;
	uint16 mExpectedSeq;

	//fragment-related
	bool mBuildingFrag;
	uint16 mFragStart;
	uint16 mFragEnd;
	uint16 mFragMilestone;

	ReadPacket* mFuturePackets[SEQUENCE_MAX];
	Packet* mSentPackets[SEQUENCE_MAX];

	std::queue<ReadPacket*> mReadPacketQueue;

private:
	enum PacketSequence
	{
		SEQUENCE_PAST = -1,
		SEQUENCE_PRESENT = 0,
		SEQUENCE_FUTURE = 1
	};

	static PacketSequence compareSequence(uint16 got, uint16 expected);

public:
	AckManager(Socket* socket);

	uint16 getNextSequence() { return ++mNextSeq; }

	void setCRCKey(uint32 crc) { mCRCKey = crc; }
	uint32 getCRCKey() { return mCRCKey; }
	uint32 getSessionID() { return mSessionID; }

	void receiveAck(uint16 seq);
	void sendAck(uint16 seq);
	void sendKeepAliveAck();
	void checkInboundPacket(byte* packet, uint32 len);
	void checkInboundFragment(byte* packet, uint32 len);
	void checkFragmentComplete();
	void checkAfterPacket();
	void recordSentPacket(const Packet& packet, uint16 seq);
	void queueRawPacket(byte* data, uint32 len);

	void sendSessionRequest();
	void sendSessionDisconnect();
	void sendMaxTimeoutLengthRequest();

	std::queue<ReadPacket*>& getPacketQueue() { return mReadPacketQueue; }
	bool hasQueuedPackets() { return !mReadPacketQueue.empty(); }
};

#endif
