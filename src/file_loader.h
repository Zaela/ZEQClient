
#ifndef _ZEQ_FILE_LOADER_H
#define _ZEQ_FILE_LOADER_H

#include <unordered_map>
#include <string>

#include "types.h"
#include "memory_stream.h"
#include "file_stream.h"
#include "s3d.h"
#include "wld.h"
#include "zon.h"

class FileLoader
{
private:
	std::string mPathToEQ;

	std::unordered_map<std::string, S3D*> mS3Ds;
	std::unordered_map<std::string, WLD*> mWLDs;

public:
	void setPathToEQ(std::string path);
	std::string getPathToEQ() { return mPathToEQ; }

	//omit file extension
	S3D* getS3D(std::string name);
	void unloadS3D(std::string name);
	//omit file extension
	WLD* getWLD(std::string name, const char* fromS3D = nullptr, bool cache = true); //no point in caching zone wlds
	//emit file extension
	ZON* getZON(std::string name);

	void handleGlobalLoad();
	void handleZoneChr(std::string shortname);
};

#endif
