
#ifndef _ZEQ_CONNECTION_H
#define _ZEQ_CONNECTION_H

#include <string>

#include "types.h"
#include "socket.h"
#include "ack_manager.h"
#include "packet_receiver.h"

struct ServerListing
{
	std::string ip;
	uint32 listID;
	uint32 runtimeID;
	std::string longname;
	std::string language;
	std::string region;
	uint32 status;
	uint32 playerCount;
};

class Connection : public Socket
{
protected:
	AckManager* mAckMgr;
	PacketReceiver* mPacketReceiver;

private:
	uint32 mCRCKey;
	uint32 mAccountID;
	std::string mSessionKey;
	ServerListing* mServer;

protected:
	void inheritSession(Connection* con)
	{
		setAccountID(con->getAccountID());
		setSessionKey(con->getSessionKey());
		setServer(con->getServer());
	}

public:
	Connection(const char* ip, uint16 port, bool isLogin = false) :
		Socket(ip, port),
		mServer(nullptr)
	{
		mAckMgr = new AckManager(this);
		mPacketReceiver = new PacketReceiver(this, mAckMgr, isLogin);
	}

	virtual ~Connection()
	{
		delete mPacketReceiver;
		delete mAckMgr;
	}

	void initiateConnection()
	{
		//send a session request to initiate with the server
		mAckMgr->sendSessionRequest();
		//wait for response
		int len = recvWithTimeout(5000);
		mPacketReceiver->handleProtocol(len);
		//increase our timeout window from 1.5 seconds to 5 so we don't have to spam quite so much...
		mAckMgr->sendMaxTimeoutLengthRequest();

		mCRCKey = mAckMgr->getCRCKey();
	}

	uint32 getCRCKey() { return mCRCKey; }
	uint32 getAccountID() { return mAccountID; }
	void setAccountID(uint32 id) { mAccountID = id; }
	std::string getSessionKey() { return mSessionKey; }
	void setSessionKey(std::string key) { mSessionKey = key; }
	ServerListing* getServer() { return mServer; }
	void setServer(ServerListing* server) { mServer = server; }
};

#endif
