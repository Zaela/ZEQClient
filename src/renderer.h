
#ifndef _ZEQ_RENDERER_H
#define _ZEQ_RENDERER_H

#include "types.h"

#include <irrlicht.h>
#include <FreeImage.h>
#include "rocket.h"

#include <string>
#include <vector>

#include "memory_stream.h"
#include "input.h"
#include "exception.h"
#include "zone_model.h"
#include "camera.h"
#include "animated_texture.h"
#include "mob_manager.h"
#include "wld_skeleton.h"
#include "zeq_lua.h"

using namespace irr;

class Renderer
{
private:
	static const uint32 SLEEP_TIME_DEFAULT = 20; //milliseconds

private:
	IrrlichtDevice* mDevice;
	video::IVideoDriver* mDriver;
	scene::ISceneManager* mSceneMgr;
	scene::ISceneCollisionManager* mCollisionMgr;

	RocketRenderer* mGUIRenderer;
	Rocket::Core::Context* mGUIContext;
	Rocket::Core::ElementDocument* mGUIDocument;

	uint32 mSleepMilliseconds;
	uint32 mPrevTime;

	scene::IMeshSceneNode* mCollisionNode;
	scene::ITriangleSelector* mCollisionSelector;
	ZoneModel* mActiveZoneModel;

	std::vector<AnimatedTexture> mAnimatedTextures;

private:
	static IrrlichtDevice* createDevice(SIrrlichtCreationParameters& params, std::string selectedRenderer);

public:
	Renderer();
	
	void initialize();
	void initializeGUI();
	void close();

	void sleep(uint32 milliseconds) { mDevice->sleep(milliseconds); }

	bool isOpenGL() { return mDriver->getDriverType() == video::EDT_OPENGL; }
	bool isDirectX() { return mDriver->getDriverType() == video::EDT_DIRECT3D9; }

	video::ITexture* createTexture(MemoryStream* file, std::string name, bool& isDDS);
	video::ITexture* createTexture(std::string name, void* pixels, uint32 width, uint32 height, bool own_pixels = true);
	void destroyTexture(video::ITexture* tex);
	Camera* createCamera(bool bind = true);
	scene::ISceneCollisionManager* getCollisionManager() { return mCollisionMgr; }
	scene::IMeshSceneNode* getCollisionNode() { return mCollisionNode; }
	scene::ITriangleSelector* getCollisionSelector() { return mCollisionSelector; }

	scene::ISceneManager* getSceneManager() { return mSceneMgr; }
	video::IVideoDriver* getVideoDriver() { return mDriver; }
	Rocket::Core::Context* getGUIContext() { return mGUIContext; }
	Rocket::Core::ElementDocument* getGUIDocument() { return mGUIDocument; }

	float loopStep();
	void resetInternalTimer();
	void useZoneModel(ZoneModel* zoneModel);
	void checkAnimatedTextures(uint32 delta);

	static scene::SMesh* copyMesh(scene::SMesh* mesh);

	enum GUIType
	{
		GUI_SPLASH,
		GUI_CREDENTIALS,
		GUI_SERVER_LIST,
		GUI_CHAR_SELECT,
		GUI_ZONE,
		GUI_VIEWER
	};

	void loadGUI(GUIType guiType);
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
