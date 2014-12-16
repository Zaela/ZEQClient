
#include "compression.h"

//these are very much single-threaded, would need to change if used in a concurrent context

static const unsigned long BUFFER_LEN = 16384;
static byte BUFFER[BUFFER_LEN];

namespace Compression
{
	bool decompressBlock(byte*& data, uint32& len, uint32 buffer_offset)
	{
		unsigned long buflen = BUFFER_LEN - buffer_offset;
		if (uncompress(BUFFER + buffer_offset, &buflen, data, len) != 0)
			return false;

		//note that we are returning the static decompress buffer
		data = BUFFER;
		len = (uint32)buflen;
		return true;
	}

	bool decompressPacket(byte*& data, uint32& len)
	{
		//data is Socket.mRecvBuf; if decompressed, Compression::BUFFER; no delete in either case
		if (len > 3 && data[2] == 'Z')
		{
			uint16 opcode = *(uint16*)data;
			//skip the opcode and compression flag
			data += 3;
			len -= 3;
			//decompress, leaving the first 2 bytes of the buffer clear
			if (!decompressBlock(data, len, 2))
				return false;
			//write the opcode back to the first 2 bytes
			*(uint16*)data = opcode;
			len += 2;
		}
		return true;
	}

	void compressBlock(byte*& data, uint32& len)
	{
		if (compressBound(len) > BUFFER_LEN)
			throw ZEQException("Compression::compressBlock: compression failed - compressBound too long");

		unsigned long buflen = BUFFER_LEN;
		if (compress2(BUFFER, &buflen, data, len, 9) != 0)
			throw ZEQException("Compression::compressBlock: compression failed");

		data = BUFFER;
		len = (uint32)buflen;
	}
}
