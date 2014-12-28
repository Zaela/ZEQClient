
#include "eqstr.h"

static std::unordered_map<int, std::string> EQStrTable;
static const uint32 MAX_FORMATS = 9;

namespace EQStr
{
	void initialize(std::string eqpath)
	{
		eqpath += "eqstr_us.txt";
		FileStream file(eqpath.c_str());
		
		const char* data = (const char*)file.getData();
		const uint32 totalLen = file.length();

		uint32 p = 0;
		auto skipLine = [=, &p]()
		{
			while (p < totalLen)
			{
				if (data[p++] == '\n')
					break;
			}
		};

		//skip the first line "EQST0002"
		skipLine();
		
		//next line contains "0 ", followed by the number of entries in the file
		p += 2; //skip "0 "
		const int num_entries = atoi((char*)(data + p));
		skipLine();

		EQStrTable.reserve(num_entries);

		//next line has all the possible format specifiers (%1 to %9) but we already know them so we don't care
		skipLine();

		//now onto the entries
		//each entry takes the form "<id#> <string>\n"
		int n = 0;
		while (p < totalLen && n++ < num_entries)
		{
			int id = atoi((char*)(data + p));
			while (p < totalLen)
			{
				if (data[p++] == ' ')
					break;
			}

			//from p to the next \n is our string
			const char* str = (char*)(data + p);
			uint16 len = 0;
			while (p < totalLen)
			{
				if (data[p++] == '\n')
					break;
				++len;
			}

			EQStrTable[id] = std::string(str, len);
		}
	}

	void formatString(std::string& out, int id, const char* string_block)
	{
		if (EQStrTable.count(id) == 0)
			return;

		const char* strings[MAX_FORMATS];
		uint32 lengths[MAX_FORMATS];
		memset(strings, 0, sizeof(const char*) * MAX_FORMATS);
		memset(lengths, 0, sizeof(uint32) * MAX_FORMATS);

		//collect format input strings
		const char* ptr = string_block;
		for (uint32 i = 0; i < MAX_FORMATS; ++i)
		{
			if (*ptr == 0)
				break;
			strings[i] = ptr;
			lengths[i] = strlen(ptr);
			ptr += lengths[i] + 1;
		}

		std::string& fmt = EQStrTable[id];
		//calculate the exact length our output string will have, to avoid reallocations
		uint32 len = 0;
		for (uint32 i = 0; i < fmt.length(); ++i)
		{
			if (fmt[i] == '%')
			{
				++i;
				if (isdigit(fmt[i]))
				{
					len += lengths[fmt[i] - '1'];
					continue;
				}
			}
			++len;
		}

		out.reserve(len);
		//write format string
		for (uint32 i = 0; i < fmt.length(); ++i)
		{
			if (fmt[i] == '%')
			{
				++i;
				if (isdigit(fmt[i]))
				{
					int n = fmt[i] - '1';
					if (strings[n])
						out += strings[n];
					continue;
				}
			}
			out += fmt[i];
		}
	}
}
