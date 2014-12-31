
#ifndef _ZEQ_COMPRESSION_H
#define _ZEQ_COMPRESSION_H

#include <zlib.h>

#include "types.h"
#include "packet.h"
#include "exception.h"

namespace Compression
{
	bool decompressBlock(byte*& data, uint32& len, uint32 buffer_offset = 0);
	bool decompressPacket(byte*& packet, uint32& len);
	void compressBlock(byte*& data, uint32& len);
}

#endif
