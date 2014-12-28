
#ifndef _ZEQ_EQSTR_H
#define _ZEQ_EQSTR_H

#include <string>
#include <unordered_map>

#include "types.h"
#include "file_stream.h"

namespace EQStr
{
	void initialize(std::string eqpath);
	void formatString(std::string& outstring, int id, const char* strings_block);
}

#endif
