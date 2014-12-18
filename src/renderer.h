
#ifndef _ZEQ_RENDERER_H
#define _ZEQ_RENDERER_H

#include "types.h"

#include <irrlicht.h>
#include <FreeImage.h>

#include <string>
#include <vector>

#include "memory_stream.h"
#include "input.h"
#include "exception.h"
#include "zone_model.h"
#include "camera.h"
#include "animated_texture.h"

using namespace irr;

class Renderer
{
private:
	IrrlichtDevice* mDevice;
	video::IVideoDriver* mDriver;
	scene::ISceneManager* mSceneMgr;
	scene::ISceneCollisionManager* mCollisionMgr;
	uint32 mSleepMilliseconds;
	uint32 mPrevTime;

	scene::IMeshSceneNode* mCollisionNode;
	scene::ITriangleSelector* mCollisionSelector;
	ZoneModel* mActiveZoneModel;

	std::vector<AnimatedTexture> mAnimatedTextures;

private:
	static IrrlichtDevice* createDevice(SIrrlichtCreationParameters& params);

public:
	Renderer();
	
	void initialize();
	void close();

	bool isOpenGL() { return mDriver->getDriverType() == video::EDT_OPENGL; }

	video::ITexture* createTexture(MemoryStream* file, std::string name, bool& isDDS);
	void destroyTexture(video::ITexture* tex);
	Camera* createCamera(bool bind = true);
	scene::ISceneCollisionManager* getCollisionManager() { return mCollisionMgr; }
	scene::IMeshSceneNode* getCollisionNode() { return mCollisionNode; }
	scene::ITriangleSelector* getCollisionSelector() { return mCollisionSelector; }

	float loopStep();
	void useZoneModel(ZoneModel* zoneModel);
	void checkAnimatedTextures(uint32 delta);
};


//to make Irrlicht happy about loading textures
class IrrTextureFile : public io::IReadFile
{
private:
	io::path mName;
	byte* mData;
	long mLength;
	long mPos;

public:
	IrrTextureFile(const char* name, byte* data, uint32 len) :
		mName(name), mData(data), mLength(len), mPos(0)
	{

	}

	~IrrTextureFile()
	{

	}

	virtual const io::path& getFileName() const override
	{
		return mName;
	}

	virtual long getPos() const override
	{
		return mPos;
	}

	virtual long getSize() const override
	{
		return mLength;
	}

	virtual int32 read(void* buffer, uint32 sizeToRead) override
	{
		long read = mPos + sizeToRead;
		if (read >= mLength)
			sizeToRead = mLength - mPos;
		memcpy(buffer, &mData[mPos], sizeToRead);
		mPos = read;
		return sizeToRead;
	}

	virtual bool seek(long finalPos, bool relativeMovement = false) override
	{
		if (relativeMovement)
			finalPos += mPos;
		if (finalPos >= mLength)
			return false;
		mPos = finalPos;
		return true;
	}
};

#endif
