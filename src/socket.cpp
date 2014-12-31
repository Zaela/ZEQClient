
#include "socket.h"

Socket::Socket(const char* ip, uint16 port) : mSocket(INVALID_SOCKET)
{
	addrinfo addr;
	memset(&addr, 0, sizeof(addrinfo));
	addr.ai_family = AF_INET;
	addr.ai_socktype = SOCK_DGRAM;

	addrinfo* list;

	char port_buf[12];
	itoa(port, port_buf, 10);

	int err = getaddrinfo(ip, port_buf, &addr, &list);
	if (err != 0)
		throw ZEQException("Socket::Socket: getaddrinfo failed: %i %s", err, gai_strerror(err));

	SOCKET sock = INVALID_SOCKET;
	for (addrinfo* res = list; res != nullptr; res = res->ai_next)
	{
		//get a socket
		sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sock == INVALID_SOCKET)
			continue;
		//set reuseaddr
		char reuse[1] = {1};
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reuse, 1) != 0)
			continue;
		//connect
		if (connect(sock, res->ai_addr, res->ai_addrlen) != 0)
			continue;
		//set non-blocking
#ifdef _WIN32
		unsigned long nonblock[1] = {1};
		if (ioctlsocket(sock, FIONBIO, nonblock) == 0)
#else
		if (fcntl(sock, F_SETFL, O_NONBLOCK) == 0)
#endif
			break; //success
		//if we're still here, we're connected but setting non-blocking failed, need to disconnect
#ifdef _WIN32
		closesocket(sock);
#else
		close(sock);
#endif
		sock = INVALID_SOCKET; //in case this is the last round
	}

	freeaddrinfo(list);

	if (sock == INVALID_SOCKET)
		throw ZEQException("Socket::Socket: could not create socket");

	mSocket = sock;
}

Socket::~Socket()
{
	if (mSocket != INVALID_SOCKET)
#ifdef _WIN32
		closesocket(mSocket);
#else
		close(mSocket);
#endif
}

int Socket::recvPacket()
{
	int i = recv(mSocket, (char*)mRecvBuf, RECV_BUF_SIZE, 0);

	if (i > 0)
	{
		return i;
	}
	else if (i == -1)
	{
		int err;
#ifdef _WIN32
		err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
#else
		err = errno;
		if (err == EWOULDBLOCK)
#endif
			return i;
		else
			throw ZEQException("Socket::recvPacket: error %i", err);
	}
	return -1;
}

int Socket::recvWithTimeout(uint32 milliseconds)
{
	uint32 cycles = milliseconds / 20;
	for (uint32 i = 0; i < cycles; ++i)
	{
		int len = recvPacket();
		if (len > 0)
			return len;
#ifdef _WIN32
		Sleep(20);
#else
		usleep(20 * 1000);
#endif
	}
	return 0;
}

void Socket::sendPacket(void* in_data, int len)
{
	char* data = (char*)in_data;
	int sent;
	do
	{
		sent = send(mSocket, data, len, 0);

		if (sent > 0)
		{
			len -= sent;
			data += sent;
		}
		else if (sent == -1)
		{
			int err;
#ifdef _WIN32
			err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK)
#else
			err = errno;
			if (err == EWOULDBLOCK)
#endif
				continue;
			else
				throw ZEQException("Socket::sendPacket: error %i", err);
		}
	}
	while (len > 0);
}

void Socket::loadLibrary()
{
#ifdef _WIN32
	WSAData wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		throw ZEQException("Socket::loadLibrary: could not start WinSock");
#endif
}

void Socket::closeLibrary()
{
#ifdef _WIN32
	WSACleanup();
#endif
}
