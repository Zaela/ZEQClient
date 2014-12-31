
#include "renderer.h"

extern Input gInput;
extern MobManager gMobMgr;

using namespace irr;

Renderer::Renderer() :
	mDevice(nullptr),
	mDriver(nullptr),
	mSceneMgr(nullptr),
	mCollisionMgr(nullptr),
	mGUIDocument(nullptr),
	mSleepMilliseconds(SLEEP_TIME_DEFAULT),
	mPrevTime(0),
	mCollisionNode(nullptr),
	mActiveZoneModel(nullptr)
{

}

void Renderer::initialize()
{
	SIrrlichtCreationParameters p;
	p.Vsync = Lua::getConfigBool(CONFIG_VAR_VSYNC, false);
	p.WindowSize = core::dimension2du(
		Lua::getConfigInt(CONFIG_VAR_SCREEN_WIDTH, 640),
		Lua::getConfigInt(CONFIG_VAR_SCREEN_HEIGHT, 480)
	);
	p.Fullscreen = Lua::getConfigBool(CONFIG_VAR_FULLSCREEN, false);

	p.EventReceiver = &gInput;

	mDevice = createDevice(p, Lua::getConfigString(CONFIG_VAR_RENDERER, ""));

	if (mDevice)
	{
		mDevice->setWindowCaption(L"ZEQClient");
		
		mDriver = mDevice->getVideoDriver();
		mSceneMgr = mDevice->getSceneManager();
		mCollisionMgr = mSceneMgr->getSceneCollisionManager();

		//rocket
		mGUIRenderer->setIsDirectX(isDirectX());
		mGUIContext = Lua::initGUI(
			Lua::getConfigInt(CONFIG_VAR_SCREEN_WIDTH, 640),
			Lua::getConfigInt(CONFIG_VAR_SCREEN_HEIGHT, 480)
		);

		gInput.setGUIContext(mGUIContext);
	}
}

void Renderer::initializeGUI()
{
	mGUIRenderer = new RocketRenderer;
	Rocket::Core::SetRenderInterface(mGUIRenderer);
	Rocket::Core::SetSystemInterface(new RocketSystem);
	Rocket::Core::Initialise();
	Rocket::Controls::Initialise();
	Rocket::Core::Lua::Interpreter::Initialise(Lua::getState());
	Rocket::Controls::Lua::RegisterTypes(Lua::getState());

	Lua::loadFontsGUI();
}

void Renderer::close()
{
	if (mDevice)
	{
		mDevice->drop();
		mDevice = nullptr;
		mDriver = nullptr;
		mSceneMgr = nullptr;
		mCollisionMgr = nullptr;
	}
}

IrrlichtDevice* Renderer::createDevice(SIrrlichtCreationParameters& params, std::string selectedRenderer)
{
	IrrlichtDevice* device = nullptr;
	//try user-selected
	if (selectedRenderer.size())
	{
		if (selectedRenderer.compare("OpenGL") == 0)
		{
			params.DriverType = video::EDT_OPENGL;
			device = createDeviceEx(params);
		}
		else if (selectedRenderer.compare("DirectX") == 0)
		{
			params.DriverType = video::EDT_DIRECT3D9;
			device = createDeviceEx(params);
		}
		else if (selectedRenderer.compare("Software") == 0)
		{
			params.DriverType = video::EDT_SOFTWARE;
			device = createDeviceEx(params);
		}

		if (device)
			return device;
	}

	//try DirectX
	params.DriverType = video::EDT_DIRECT3D9;
	device = createDeviceEx(params);

	if (device)
		return device;

	//try OpenGL
	params.DriverType = video::EDT_OPENGL;
	device = createDeviceEx(params);

	if (device)
		return device;

	throw ZEQException("Renderer::createDevice: could not create render device");
}

video::ITexture* Renderer::createTexture(MemoryStream* file, std::string name, bool& isDDS)
{
	byte* data = file->getData();
	unsigned long len = file->length();

	FIMEMORY* fi_mem = FreeImage_OpenMemory(data, len);
	FREE_IMAGE_FORMAT fmt = FreeImage_GetFileTypeFromMemory(fi_mem);

	video::ITexture* tex = nullptr;
	//Irrlicht doesn't have a built-in DDS loader
	if (fmt == FIF_DDS)
	{
		isDDS = true;
		FIBITMAP* bitmap = FreeImage_LoadFromMemory(fmt, fi_mem);
		video::IImage* img = mDriver->createImageFromData(video::ECF_A8R8G8B8,
			core::dimension2d<uint32>(FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap)),
			FreeImage_GetBits(bitmap), false, true);
		FreeImage_Unload(bitmap);

		tex = mDriver->addTexture(name.c_str(), img);
	}
	else if (fmt == FIF_BMP)
	{
		//need to process bitmaps to add an alpha channel in case they are masked
		FIBITMAP* bitmap = FreeImage_LoadFromMemory(fmt, fi_mem);

		const uint32 w = FreeImage_GetWidth(bitmap);
		const uint32 h = FreeImage_GetHeight(bitmap);

		//the first pixel (top left) is usually the mask color, is it always?
		byte p;
		RGBQUAD* palette = FreeImage_GetPalette(bitmap);
		FreeImage_GetPixelIndex(bitmap, 0, 0, &p);
		RGBQUAD trans = palette[p];

		video::IImage* img = mDriver->createImage(video::ECF_A8R8G8B8, core::dimension2d<uint32>(w, h));
		video::SColor clr;
		RGBQUAD c;
		for (uint32 x = 0; x < w; ++x)
		{
			for (uint32 y = 0; y < h; ++y)
			{
				FreeImage_GetPixelIndex(bitmap, x, y, &p);
				c = palette[p];
				clr.set((c.rgbRed == trans.rgbRed && c.rgbGreen == trans.rgbGreen && c.rgbBlue == trans.rgbBlue) ? 0 : 255,
					c.rgbRed, c.rgbGreen, c.rgbBlue);
				img->setPixel(x, h - y - 1, clr);
			}
		}

		FreeImage_Unload(bitmap);
		tex = mDriver->addTexture(name.c_str(), img);
	}
	else
	{
		IrrTextureFile* file = new IrrTextureFile(name.c_str(), data, len);
		tex = mDriver->getTexture(file);
		file->drop();
	}

	FreeImage_CloseMemory(fi_mem);

	return tex;
}

video::ITexture* Renderer::createTexture(std::string name, void* pixels, uint32 width, uint32 height, bool own_pixels)
{
	video::IImage* img = mDriver->createImageFromData(video::ECF_A8R8G8B8,
		core::dimension2du(width, height), pixels, own_pixels, true);
	video::ITexture* tex = mDriver->addTexture(name.c_str(), img);
	return tex;
}

void Renderer::destroyTexture(video::ITexture* tex)
{
	mDriver->removeTexture(tex);
}

Camera* Renderer::createCamera(bool bind)
{
	scene::ICameraSceneNode* node = mSceneMgr->addCameraSceneNode();
	node->bindTargetAndRotation(bind);
	return new Camera(node);
}

float Renderer::loopStep()
{
	if (mDevice->run() && mDevice->isWindowActive() && mDevice->isWindowFocused())
	{
		mDriver->beginScene(true, true, video::SColor(255, 128, 128, 128));
		mSceneMgr->drawAll();

		mGUIContext->Update();
		mGUIContext->Render();

		mDriver->endScene();

		mDevice->sleep(mSleepMilliseconds);
	}
	else
	{
		mDevice->sleep(100);
	}

	uint32 time = mDevice->getTimer()->getTime();
	uint32 delta = time - mPrevTime;
	mPrevTime = time;

	//animated textures
	if (!mAnimatedTextures.empty())
		checkAnimatedTextures(delta);

	float delta_f = (float)delta * 0.001f;

	//wld skeleton animation
	gMobMgr.animateNearbyMobs(delta_f);

	return delta_f;
}

void Renderer::resetInternalTimer()
{
	mDevice->getTimer()->setTime(0);
}

void Renderer::useZoneModel(ZoneModel* zoneModel)
{
	mSceneMgr->clear();
	if (isOpenGL())
		mSceneMgr->getParameters()->setAttribute(scene::ALLOW_ZWRITE_ON_TRANSPARENT, true); 

	//animated textures
	std::vector<AnimatedTexture> animTexturesTemp;
	for (const AnimatedTexture& animTex : zoneModel->getAnimatedTextures())
	{
		animTexturesTemp.push_back(animTex);
	}

	//main zone geometry
	core::vector3df pos(zoneModel->getX(), zoneModel->getY(), zoneModel->getZ());
	mCollisionNode = mSceneMgr->addOctreeSceneNode(zoneModel->getMesh());
	mCollisionNode->setPosition(pos);
	scene::ITriangleSelector* sel = mSceneMgr->createOctreeTriangleSelector(zoneModel->getMesh(), mCollisionNode);
	mCollisionNode->setTriangleSelector(sel);
	mCollisionSelector = sel;
	sel->drop();

	scene::IMeshSceneNode* noncollision_node = mSceneMgr->addOctreeSceneNode(zoneModel->getNonCollisionMesh());
	noncollision_node->setPosition(pos);

	mSceneMgr->setAmbientLight(video::SColorf(1, 1, 1, 1));
	mCollisionNode->setPosition(core::vector3df(zoneModel->getX(), zoneModel->getY(), zoneModel->getZ()));

	//update animated texture with target scene node, if applicable
	scene::IMesh* mesh = zoneModel->getMesh()->getMesh(0);
	scene::IMesh* noncollision_mesh = zoneModel->getNonCollisionMesh()->getMesh(0);
	for (AnimatedTexture& animTex : animTexturesTemp)
	{
		if (animTex.replaceMeshWithSceneNode(mesh, mCollisionNode) ||
			animTex.replaceMeshWithSceneNode(noncollision_mesh, noncollision_node))
			mAnimatedTextures.push_back(animTex);
	}

	//placed objects
	for (const ObjectPlacement& obj : zoneModel->getObjectPlacements())
	{
		scene::IAnimatedMeshSceneNode* objNode = mSceneMgr->addAnimatedMeshSceneNode(obj.mesh, mCollisionNode, -1,
			core::vector3df(obj.x, obj.y, obj.z),
			core::vector3df(obj.rotX, obj.rotY, obj.rotZ),
			core::vector3df(obj.scaleX, obj.scaleY, obj.scaleZ));

		if (obj.collidable)
		{
			sel = mSceneMgr->createTriangleSelector(objNode);
			objNode->setTriangleSelector(sel);
			sel->drop();
		}

		//update animated texture with target scene node, if applicable
		scene::IMesh* mesh = obj.mesh->getMesh(0);
		for (AnimatedTexture& animTex : animTexturesTemp)
		{
			if (animTex.checkMesh(mesh))
			{
				AnimatedTexture copy = animTex;
				copy.setMeshPtr(objNode);
				mAnimatedTextures.push_back(copy);
			}
		}
	}

	mActiveZoneModel = zoneModel;
}

void Renderer::checkAnimatedTextures(uint32 delta)
{
	for (AnimatedTexture& animTex : mAnimatedTextures)
	{
		animTex.time += delta;
		if (animTex.time >= animTex.frame_delay)
		{
			animTex.advanceFrame();
			animTex.time -= animTex.frame_delay;
		}
	}
}

scene::SMesh* Renderer::copyMesh(scene::SMesh* mesh)
{
	scene::SMesh* copy = new scene::SMesh;

	for (uint32 i = 0; i < mesh->getMeshBufferCount(); ++i)
	{
		scene::SMeshBuffer* copyBuf = new scene::SMeshBuffer;
		scene::IMeshBuffer* buf = mesh->getMeshBuffer(i);

		copyBuf->Material = buf->getMaterial();

		//making space and memcpying the data directly doesn't work for whatever reason
		video::S3DVertex* verts = (video::S3DVertex*)buf->getVertices();
		for (uint32 j = 0; j < buf->getVertexCount(); ++j)
			copyBuf->Vertices.push_back(verts[j]);

		uint16* indices = buf->getIndices();
		for (uint32 j = 0; j < buf->getIndexCount(); ++j)
			copyBuf->Indices.push_back(indices[j]);

		copyBuf->recalculateBoundingBox();
		copy->addMeshBuffer(copyBuf);
	}
	copy->recalculateBoundingBox();

	return copy;
}

void Renderer::loadGUIDocument(Rocket::Core::String path)
{
	if (mGUIDocument)
		mGUIContext->UnloadDocument(mGUIDocument);

	mGUIDocument = mGUIContext->LoadDocument(path);
	if (mGUIDocument)
	{
		mGUIDocument->SetId(path);
		mGUIDocument->Show();
		mGUIDocument->RemoveReference();
	}
}
