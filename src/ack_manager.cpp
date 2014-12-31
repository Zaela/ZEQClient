
#include "ack_manager.h"

extern Random gRNG;

AckManager::AckManager(Socket* socket) : 
	mSocket(socket),
	mNextSeq(65535),
	mExpectedSeq(0),
	mLastReceivedAck(65535),
	mBuildingFrag(false),
	mFragEndReceived(false)
{
	memset(mFuturePackets, 0, sizeof(ReadPacket*) * SEQUENCE_MAX);
	memset(mSentPackets, 0, sizeof(Packet*) * SEQUENCE_MAX);
}

AckManager::PacketSequence AckManager::compareSequence(uint16 got, uint16 expected)
{
	if (got == expected)
		return SEQUENCE_PRESENT;

	if ((got > expected && got < expected + WINDOW_SIZE) || got < (expected - WINDOW_SIZE))
		return SEQUENCE_FUTURE;

	return SEQUENCE_PAST;
}

void AckManager::receiveAck(uint16 seq)
{
	mLastReceivedAck = seq;

	for (uint16 count = 0; count < WINDOW_SIZE; ++count)
	{
		if (mSentPackets[seq])
		{
			delete mSentPackets[seq];
			mSentPackets[seq] = nullptr;
		}
		else
			break;
		--seq;
	}
}

void AckManager::sendAck(uint16 seq)
{
	static Packet* ackPacket = new Packet(2, OP_NONE, nullptr, OP_Ack, false, false);
	ackPacket->setSequence(seq);
	ackPacket->send(mSocket, mCRCKey);
}

void AckManager::sendKeepAliveAck()
{
	sendAck(mExpectedSeq - 1);
}

void AckManager::checkInboundPacket(byte* packet, uint32 len)
{
	uint16 seq = toHostShort(*(uint16*)(packet + 2));

	switch (compareSequence(seq, mExpectedSeq))
	{
	case SEQUENCE_PRESENT:
	{
		//this is our next expected packet, queue it
		mReadPacketQueue.push(new ReadPacket(packet + 4, len - 4)); //may want to handle the +4 -4 at the end for all packets equally to avoid intermediate allocations
		++mExpectedSeq;
		//check if we have any packets ahead of this one ready to be processed
		checkAfterPacket();

		sendAck(mExpectedSeq - 1);
		break;
	}
	case SEQUENCE_FUTURE:
	{
		//future packet: remember it for later
		mFuturePackets[seq] = new ReadPacket(packet, len);
		break;
	}
	case SEQUENCE_PAST:
		//sendOutOfOrderRequest(seq);
		break;
	}
}

void AckManager::checkInboundFragment(byte* packet, uint32 len)
{
	uint16 seq = toHostShort(*(uint16*)(packet + 2));

	switch (compareSequence(seq, mExpectedSeq))
	{
	case SEQUENCE_PRESENT:
	{
		//this is the starting packet of a fragment sequence
		startFragSequence(packet, seq);
		mFuturePackets[seq] = new ReadPacket(packet, len);

		if (mFragEndReceived)
			checkFragmentComplete();
	}
	case SEQUENCE_FUTURE:
	{
		//future packet: remember it for later
		mFuturePackets[seq] = new ReadPacket(packet, len);

		if (!mFragEndReceived && seq == (mFragEnd - 1))
			mFragEndReceived = true;

		if (mBuildingFrag)
		{
			if (mFragEndReceived)
				checkFragmentComplete();
			//if we didn't finish a packet...
			if (mBuildingFrag && (seq - mFragMilestone) >= 10)
			{
				mFragMilestone = seq;
				sendAck(seq);
			}
		}
		break;
	}
	case SEQUENCE_PAST:
		//sendOutOfOrderRequest(seq);
		break;
	}
}

void AckManager::checkFragmentComplete()
{
	ReadPacket* rp = mFuturePackets[mFragStart];
	if (rp == nullptr)
		return;

	uint32 len = rp->len - 8;
	uint16 i = mFragStart + 1;
	while (i != mFragEnd)
	{
		rp = mFuturePackets[i];
		if (rp == nullptr)
			return;
		len += rp->len - 4;
		++i;
	}

	//if we're still here, we had a complete fragment sequence, and we know how long it is
	ReadPacket* out = new ReadPacket(nullptr, len);

	//copy first piece
	ReadPacket* first = mFuturePackets[mFragStart];
	mFuturePackets[mFragStart] = nullptr;
	uint32 copy_len = first->len - 8;
	memcpy(out->data, first->data + 8, copy_len);
	delete first;
	uint32 pos = copy_len;

	//copy subsequence pieces
	i = mFragStart + 1;
	while (i != mFragEnd)
	{
		ReadPacket* sub = mFuturePackets[i];
		mFuturePackets[i] = nullptr;
		copy_len = sub->len - 4;
		memcpy(out->data + pos, sub->data + 4, copy_len);
		delete sub;
		pos += copy_len;
		++i;
	}

	//add to queue
	mReadPacketQueue.push(out);

	//clean up
	mExpectedSeq = i;
	mBuildingFrag = false;
	mFragEndReceived = false;
	sendAck(i - 1);

	checkAfterPacket();
	if (mExpectedSeq != i)
		sendAck(mExpectedSeq - 1);
}

void AckManager::checkAfterPacket()
{
	ReadPacket* nextPacket = mFuturePackets[mExpectedSeq];
	if (nextPacket == nullptr)
		return;

	uint16 i = mExpectedSeq;
	while (nextPacket)
	{
		uint16 opcode = toHostShort(*(uint16*)nextPacket->data);
		if (opcode == OP_Fragment)
		{
			startFragSequence(nextPacket->data, i);
			if (mFragEndReceived)
				checkFragmentComplete();
			return;
		}
		else
		{
			mReadPacketQueue.push(new ReadPacket(nextPacket->data + 4, nextPacket->len - 4));
			delete nextPacket;
			mFuturePackets[i] = nullptr;
			++mExpectedSeq;
		}

		if (i != mExpectedSeq)
		{
			i = mExpectedSeq;
			nextPacket = mFuturePackets[i];
		}
	}
}

void AckManager::recordSentPacket(const Packet& packet, uint16 seq)
{
	//copy constructor
	mSentPackets[seq] = new Packet(packet);
}

void AckManager::sendSessionRequest()
{
	mSessionID = gRNG();
	SessionRequest sr;

	sr.opcode = toNetworkShort(OP_SessionRequest);
	sr.unknown = toNetworkLong(2);
	sr.sessionID = toNetworkLong(mSessionID);
	sr.maxLength = toNetworkLong(512);

	mSocket->sendPacket(&sr, sizeof(SessionRequest));
}

void AckManager::sendSessionDisconnect()
{
	Packet packet(4, OP_NONE, nullptr, OP_SessionDisconnect, false, false);
	uint32* id = (uint32*)packet.getDataBuffer();

	*id = toNetworkLong(mSessionID);

	packet.send(mSocket, mCRCKey);
}

void AckManager::sendMaxTimeoutLengthRequest()
{
	SessionStat ss;

	ss.opcode = toNetworkShort(OP_SessionStatRequest);
	//having a high value here maxes out the timeout window to 5 seconds, so we don't have to spam acks quite so much
	ss.last_local_delta = toNetworkLong(5000000);
	//while this one decreases the amount of time the server waits between sending us strings of queued packets
	ss.average_delta = toNetworkLong(25);

	mSocket->sendPacket(&ss, sizeof(SessionStat));
}

void AckManager::queueRawPacket(byte* packet, uint32 len)
{
	ReadPacket* rp;

	if (len > 2 && packet[1] == 0xA5) //"not compressed" flag in between the two bytes of the opcode
	{
		packet[1] = packet[0];
		rp = new ReadPacket(packet + 1, len - 1);
	}
	else
	{
		rp = new ReadPacket(packet, len);
	}

	mReadPacketQueue.push(rp);
}

bool AckManager::resendUnackedPackets()
{
	int count = 0;
	uint16 i = mLastReceivedAck + 1;
	for (;;)
	{
		if (mSentPackets[i])
		{
			mSentPackets[i]->send(mSocket, mCRCKey);
			++count;
		}
		else
		{
			break;
		}
		++i;
	}

	printf("Resending unacked: %i\n", count);

	return count > 0;
}

void AckManager::startFragSequence(byte* data, uint16 seq)
{
	mBuildingFrag = true;
	mFragStart = seq;
	mFragMilestone = seq;

	//find the expected end seq
	uint32 size = toHostLong(*(uint32*)(data + 4));
	//max packet size is 512 - 6 = 506
	mFragEnd = seq + (size / 506) + 1;
	//if it's an exact multiple we just overshot it by 1
	if (size % 506 == 0)
		--mFragEnd;
	//check if we've already received the last frag
	if (mFuturePackets[mFragEnd - 1])
		mFragEndReceived = true;
}
