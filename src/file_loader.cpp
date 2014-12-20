
#include "file_loader.h"

void FileLoader::setPathToEQ(std::string path)
{
	mPathToEQ = path;
	char c = mPathToEQ.back();
#ifdef _WIN32
	if (c != '\\')
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
	try
	{
		S3D* s3d = new S3D(ext_name.c_str());
		mS3Ds[name] = s3d;
		return s3d;
	}
	catch (ZEQException&)
	{
		try
		{
			ext_name = mPathToEQ + name + ".eqg";
			S3D* s3d = new S3D(ext_name.c_str());
			mS3Ds[name] = s3d;
			return s3d;
		}
		catch (ZEQException& e)
		{
			printf("Error: %s\n", e.what());
		}
	}

	return nullptr;
}

WLD* FileLoader::getWLD(std::string name, const char* fromS3D)
{
	//some WLDs have generic names (e.g. 'objects.wld'), use fromS3D to specify source
	std::string orig_name;
	if (fromS3D)
	{
		orig_name = name;
		name = fromS3D + '/' + name;
	}

	if (mWLDs.count(name) != 0)
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
		mWLDs[name] = wld;
		return wld;
	}
	catch (ZEQException& e)
	{
		printf("Error: %s\n", e.what());
	}

	return nullptr;
}

TER* FileLoader::getTER(std::string name, std::string ter_model_name)
{
	S3D* s3d = getS3D(name);
	if (s3d == nullptr)
		return nullptr;

	MemoryStream* file = s3d->getFile(ter_model_name.c_str());
	if (file == nullptr)
		return nullptr;

	try
	{
		TER* ter = new TER(file, s3d, name);
		return ter;
	}
	catch (ZEQException& e)
	{
		printf("Error: %s\n", e.what());
	}

	return nullptr;
}
