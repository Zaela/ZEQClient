
#include "file_loader.h"

void FileLoader::setPathToEQ(std::string path)
{
	mPathToEQ = path;
	char c = mPathToEQ.back();
#ifdef _WIN32
	if (c != '\\' && c != '/')
		mPathToEQ += '\\';
#else
	if (c != '/')
		mPathToEQ += '/';
#endif
}

S3D* FileLoader::getS3D(std::string name)
{
	if (mS3Ds.count(name) != 0)
		return mS3Ds[name];

	std::string ext_name = mPathToEQ + name + ".s3d";
	
	//try .s3d, then .eqg (live client does it in the opposite order, but oh well)
	for (int i = 0; i < 2; ++i)
	{
		FileStream* file = FileStream::open(ext_name.c_str());
		if (file)
		{
			try
			{
				S3D* s3d = new S3D(file);
				mS3Ds[name] = s3d;
				return s3d;
			}
			catch (ZEQException& e)
			{
				printf("Error: %s\n", e.what());
				delete file;
			}
		}

		ext_name = mPathToEQ + name + ".eqg";
	}

	return nullptr;
}

void FileLoader::unloadS3D(std::string name)
{
	if (mS3Ds.count(name) == 0)
		return;

	delete mS3Ds[name];
	mS3Ds.erase(name);
}

WLD* FileLoader::getWLD(std::string name, const char* fromS3D, bool cache)
{
	//some WLDs have generic names (e.g. 'objects.wld'), use fromS3D to specify source
	std::string orig_name;
	if (fromS3D)
	{
		orig_name = name;
		name = fromS3D + '/' + name;
	}

	if (cache && mWLDs.count(name) != 0)
		return mWLDs[name];

	S3D* s3d = getS3D(fromS3D ? fromS3D : name);
	if (s3d == nullptr)
		return nullptr;

	std::string ext_name = fromS3D ? (orig_name + ".wld") : (name + ".wld");
	MemoryStream* file = s3d->getFile(ext_name.c_str());
	if (file == nullptr)
		return nullptr;

	try
	{
		WLD* wld = new WLD(file, s3d, name);
		if (cache)
			mWLDs[name] = wld;
		return wld;
	}
	catch (ZEQException& e)
	{
		printf("Error: %s\n", e.what());
	}

	return nullptr;
}

ZON* FileLoader::getZON(std::string name)
{
	//the zon may not be in the s3d, but we need it regardless because that's where everything else is
	S3D* s3d = getS3D(name);
	if (s3d == nullptr)
		return nullptr;

	//check inside the s3d first
	std::string ext_name = name + ".zon";
	MemoryStream* file = s3d->getFile(ext_name.c_str());
	
	for (int i = 0; i < 2; ++i)
	{
		if (file)
		{
			try
			{
				ZON* zon = new ZON(file, s3d, name, (i > 0));
				return zon;
			}
			catch (ZEQException& e)
			{
				printf("Error: %s\n", e.what());
			}
		}

		ext_name = mPathToEQ + ext_name;
		file = FileStream::open(ext_name.c_str());
	}

	//file-in-s3d won't reach here, which is good because s3ds manage their internal files themselves
	if (file)
		delete file;

	//the zon may be in the s3d under the wrong name (e.g. chambersa.zon in chambersb.eqg)
	//which is why s3ds also sort by extension for us
	uint32 n = s3d->getNumFilesWithExtension("zon");
	for (uint32 i = 0; i < n; ++i)
	{
		file = s3d->getFileByExtension("zon", i);
		if (file)
		{
			try
			{
				ZON* zon = new ZON(file, s3d, name, false);
				return zon;
			}
			catch (ZEQException& e)
			{
				printf("Error: %s\n", e.what());
			}
		}
	}

	return nullptr;
}

void FileLoader::handleGlobalLoad()
{
	//just global_chr for now
	WLD* wld = getWLD("global_chr", nullptr, false);
	wld->convertAllMobModels();
	delete wld; //don't need to keep it loaded since all the assets are extracted for sure
	unloadS3D("global_chr");
}

void FileLoader::handleZoneChr(std::string shortname)
{
	shortname += "_chr";
	WLD* wld = getWLD(shortname, nullptr, false);
	wld->convertAllMobModels();
	delete wld;
	unloadS3D(shortname);
}
