
#ifndef _ZEQ_LOGIN_CONNECTION_H
#define _ZEQ_LOGIN_CONNECTION_H

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <cstring>

#include <cryptlib.h>
#include <des.h>
#include <modes.h>
#include <filters.h>

#include "types.h"
#include "packet.h"
#include "connection.h"
#include "opcodes_login.h"
#include "exception.h"
#include "zeq_lua.h"

#define LOGIN_IP_DEFAULT "login.eqemulator.net"
#define LOGIN_PORT_DEFAULT 5998

class LoginConnection : public Connection
{
private:
	std::string mName;
	std::string mPassword;

	std::vector<ServerListing> mServerList;
	std::unordered_map<std::string, ServerListing> mServersByName;

	bool mSuccess;

private:
	struct DESEncryption
	{
		byte key[CryptoPP::DES::DEFAULT_KEYLENGTH];
		byte iv[CryptoPP::DES::BLOCKSIZE];
	};

	DESEncryption mDES;

	std::string encrypt(std::string plaintext);
	std::string decrypt(std::string ciphertext);

public:
	LoginConnection();

	bool success() { return mSuccess; }

	void setCredentials(std::string name, std::string password);
	void processInboundPackets();
	bool processPacket(uint16 opcode, byte* data, uint32 len);
	void toServerSelect();
	void quickConnect(std::string serverName);
};

#pragma pack(1)

struct Login_ReplyBlock {
	char message; //0x01
	char unknown2[7]; //0x00
	uint32 login_acct_id;
	char key[11]; //10 char + null term;
	uint32 failed_attempts;
	char unknown3[4]; //0x00, 0x00, 0x00, 0x03
	char unknown4[4]; //0x00, 0x00, 0x00, 0x02
	char unknown5[4]; //0xe7, 0x03, 0x00, 0x00
	char unknown6[4]; //0xff, 0xff, 0xff, 0xff
	char unknown7[4]; //0xa0, 0x05, 0x00, 0x00
	char unknown8[4]; //0x00, 0x00, 0x00, 0x02
	char unknown9[4]; //0xff, 0x03, 0x00, 0x00
	char unknown10[4]; //0x00, 0x00, 0x00, 0x00
	char unknown11[4]; //0x63, 0x00, 0x00, 0x00
	char unknown12[4]; //0x01, 0x00, 0x00, 0x00
	char unknown13[4]; //0x00, 0x00, 0x00, 0x00
	char unknown14[4]; //0x00, 0x00, 0x00, 0x00
};

struct Login_PlayRequest {
	uint16 sequence;
	uint32 unknown[2];
	uint32 serverRuntimeID;
};

struct Login_PlayResponse {
	uint8 sequence;
	uint8 unknown1[9];
	uint8 allowed;
	uint16 message;
	uint8 unknown2[3];
	uint32 playServerID;
};

#pragma pack()

#endif
