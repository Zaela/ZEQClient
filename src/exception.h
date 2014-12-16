
#ifndef _ZEQ_EXCEPTION_H
#define _ZEQ_EXCEPTION_H

#include <exception>
#include <cstring>
#include <cstdarg>
#include "types.h"

class ZEQException : public std::exception
{
private:
	static const size_t MAX_MSG_LEN = 1024;
	char mMsg[MAX_MSG_LEN];

public:
	ZEQException(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		vsnprintf(mMsg, MAX_MSG_LEN, fmt, args);
		va_end(args);
	}

	const char* what() const
	{
		return mMsg;
	}
};

class ZEQBasicException
{
private:
	int type;
public:
	ZEQBasicException(int t) { type = t; }

	enum Types
	{
		TIMEOUT,
		BAD_CREDENTIALS,
		NOT_FOUND,
		DISCONNECT
	};

	int getType() { return type; }

	const char* getTypeName()
	{
		switch (type)
		{
		case TIMEOUT:
			return "Timeout";
		case BAD_CREDENTIALS:
			return "Bad Credentials";
		case NOT_FOUND:
			return "Not Found";
		case DISCONNECT:
			return "Disconnect";
		default:
			return "Unknown";
		}
	}
};

class TimeoutException : public ZEQBasicException
{
public:
	TimeoutException() : ZEQBasicException(TIMEOUT) { }
};

class BadCredentialsException : public ZEQBasicException
{
public:
	BadCredentialsException() : ZEQBasicException(BAD_CREDENTIALS) { }
};

class NotFoundException : public ZEQBasicException
{
public:
	NotFoundException() : ZEQBasicException(NOT_FOUND) { }
};

class DisconnectException : public ZEQBasicException
{
public:
	DisconnectException() : ZEQBasicException(DISCONNECT) { }
};

#endif
