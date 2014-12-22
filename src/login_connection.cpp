
#include "login_connection.h"

LoginConnection::LoginConnection() :
	Connection(
		Lua::getConfigString(CONFIG_VAR_LOGIN_IP, LOGIN_IP_DEFAULT).c_str(),
		Lua::getConfigInt(CONFIG_VAR_LOGIN_PORT, LOGIN_PORT_DEFAULT), 
		true
	),
	mSuccess(false)
{
	memset(mDES.key, 0, CryptoPP::DES::DEFAULT_KEYLENGTH);
	memset(mDES.iv, 0, CryptoPP::DES::BLOCKSIZE);
}

void LoginConnection::setCredentials(std::string name, std::string password)
{
	mName = name;
	mPassword = password;
}

void LoginConnection::processInboundPackets()
{
	for (;;)
	{
		int len = recvWithTimeout(5000);
		if (!mPacketReceiver->handleProtocol(len))
			continue;
		//else we have some packets to process here
		std::queue<ReadPacket*>& queue = mAckMgr->getPacketQueue();
		while (!queue.empty())
		{
			ReadPacket* packet = queue.front();
			queue.pop();
			uint16 opcode = *(uint16*)packet->data;
			bool ret = processPacket(opcode, packet->data + 2, packet->len - 2);
			delete packet;
			if (ret)
				return;
		}
	}
}

bool LoginConnection::processPacket(uint16 opcode, byte* data, uint32 len)
{
	switch (opcode)
	{
	case OP_ChatMessage:
	{
		printf("OP_ChatMessage\n");
		//we receive this to signal that the server's ready to
		//receive login credentials, for whatever reason
		uint32 len = mName.length() + mPassword.length() + 2; //include null terminators
		std::string plaintext = mName + '\0' + mPassword + '\0';
		std::string ciphertext = encrypt(plaintext);

		Packet packet(10 + ciphertext.length(), OP_Login, mAckMgr, OP_Packet, false, false);
		byte* b = packet.getDataBuffer();
		b[0] = 3;
		b[5] = 2;
		memcpy(&b[10], ciphertext.c_str(), ciphertext.length());

		packet.send(this, mAckMgr->getCRCKey());
		break;
	}
	case OP_LoginAccepted:
	{
		printf("OP_LoginAccepted\n");
		if (len < 80) //replace this with a more appropriate exception type
			throw BadCredentialsException();
			//throw ZEQException("LoginConnection::processPacket: bad username/password");

		data += 10;
		len -= 10;

		std::string ciphertext((char*)data, len);
		std::string plaintext = decrypt(ciphertext);

		Login_ReplyBlock* rb = (Login_ReplyBlock*)plaintext.c_str();

		setAccountID(rb->login_acct_id);
		setSessionKey(rb->key);

		printf("AccountID: %u, SessionKey: %s\n", getAccountID(), getSessionKey().c_str());
		
		//send login server list request
		Packet packet(10, OP_ServerListRequest, mAckMgr, OP_Packet, false, false);
		byte* b = packet.getDataBuffer();
		b[0] = 4;

		packet.send(this, mAckMgr->getCRCKey());
		break;
	}
	case OP_ServerListResponse:
	{
		printf("OP_ServerListResponse\n");
		//uint32 count = *(uint32*)(data + 16); //we don't really need this

		uint32 offset = 20;
		while (offset < len)
		{
			ServerListing sl;
			//ip address
			sl.ip = (char*)&data[offset];
			offset += sl.ip.length() + 1;
			//listID and runtimeID
			uint32* i = (uint32*)&data[offset];
			sl.listID = *i++;
			sl.runtimeID = *i;
			offset += sizeof(uint32) * 2;
			//longname
			sl.longname = (char*)&data[offset];
			offset += sl.longname.length() + 1;
			//language
			sl.language = (char*)&data[offset];
			offset += sl.language.length() + 1;
			//region
			sl.region = (char*)&data[offset];
			offset += sl.region.length() + 1;
			//status and player count
			i = (uint32*)&data[offset];
			sl.status = *i++;
			sl.playerCount = *i;
			offset += sizeof(uint32) * 2;

			mServerList.push_back(sl);
			mServersByName[sl.longname] = sl;

			printf("Players: %u, name: %s\n", sl.playerCount, sl.longname.c_str());
		}

		return true;
	}
	case OP_PlayEverquestResponse:
	{
		Login_PlayResponse* pr = (Login_PlayResponse*)data;

		mSuccess = (pr->allowed > 0);
		if (mSuccess)
			mAckMgr->sendSessionDisconnect();
		return true;
	}
	default:
		printf("LoginConnection received unknown opcode 0x%0.4X\n", opcode);
		break;
	}

	return false;
}

void LoginConnection::toServerSelect()
{
	mAckMgr->sendSessionRequest();
	processInboundPackets();
}

void LoginConnection::quickConnect(std::string serverName)
{
	toServerSelect();

	if (mServersByName.empty())
		throw ZEQException("LoginConnection::quickConnect: bad server list");

	if (mServersByName.count(serverName) == 0)
		throw ZEQException("LoginConnection::quickConnect: could not find server");

	setServer(new ServerListing(mServersByName[serverName]));

	//inform the login server of our selection
	Packet packet(14, OP_PlayEverquestRequest, mAckMgr, OP_Packet, false, false);
	Login_PlayRequest* pr = (Login_PlayRequest*)packet.getDataBuffer();
	pr->sequence = 5;
	pr->serverRuntimeID = getServer()->runtimeID;

	packet.send(this, mAckMgr->getCRCKey());

	processInboundPackets();
}

std::string LoginConnection::encrypt(std::string plaintext)
{
	std::string ciphertext;

	CryptoPP::DES::Encryption desEncryption(mDES.key, CryptoPP::DES::DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(desEncryption, mDES.iv);

	CryptoPP::StreamTransformationFilter encryptor(cbcEncryption,
		new CryptoPP::StringSink(ciphertext),
		CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING);
	encryptor.Put((byte*)plaintext.c_str(), plaintext.length());
	encryptor.MessageEnd();

	return ciphertext;
}

std::string LoginConnection::decrypt(std::string ciphertext)
{
	std::string plaintext;

	CryptoPP::DES::Decryption desDecryption(mDES.key, CryptoPP::DES::DEFAULT_KEYLENGTH);
	CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(desDecryption, mDES.iv);

	CryptoPP::StreamTransformationFilter decryptor(cbcDecryption,
		new CryptoPP::StringSink(plaintext),
		CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING);
	decryptor.Put((byte*)ciphertext.c_str(), ciphertext.length());
	decryptor.MessageEnd();

	return plaintext;
}
