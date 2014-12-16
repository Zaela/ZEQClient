
#include "renderer.h"

extern Input gInput;

using namespace irr;

Renderer::Renderer() :
	mDevice(nullptr)
{

}

void Renderer::initialize()
{
	SIrrlichtCreationParameters p;
	p.Vsync = true;
	p.WindowSize = core::dimension2du(640, 480);

	p.EventReceiver = &gInput;

	mDevice = createDevice(p);

	if (mDevice)
	{
		mDevice->setWindowCaption(L"ZEQClient");
		
		mDriver = mDevice->getVideoDriver();
		mSceneMgr = mDevice->getSceneManager();
	}
}

void Renderer::close()
{
	if (mDevice)
		mDevice->drop();
}

IrrlichtDevice* Renderer::createDevice(SIrrlichtCreationParameters& params)
{
	//try DirectX
	params.DriverType = video::EDT_DIRECT3D9;
	IrrlichtDevice* device = createDeviceEx(params);

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
			FreeImage_GetBits(bitmap), false, false);
		FreeImage_Unload(bitmap);

		tex = mDriver->addTexture(name.c_str(), img);
	}
	else if (fmt == FIF_BMP)
	{
		//need to process bitmaps to add an alpha channel in case they are masked
		FIBITMAP* bitmap = FreeImage_LoadFromMemory(fmt, fi_mem);

		//the first pixel (top left) is usually the mask color, is it always?
		byte p;
		RGBQUAD* palette = FreeImage_GetPalette(bitmap);
		FreeImage_GetPixelIndex(bitmap, 0, 0, &p);
		RGBQUAD trans = palette[p];

		const uint32 w = FreeImage_GetWidth(bitmap);
		const uint32 h = FreeImage_GetHeight(bitmap);

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

void Renderer::loopStep()
{
	if (mDevice->run() && mDevice->isWindowActive() && mDevice->isWindowFocused())
	{
		mDriver->beginScene(true, true, video::SColor(255, 128, 128, 128));
		mSceneMgr->drawAll();
		mDriver->endScene();
	}
}

void Renderer::useZoneModel(ZoneModel* zoneModel)
{
	mSceneMgr->clear();
	scene::IMeshSceneNode* node = mSceneMgr->addOctreeSceneNode(zoneModel->getMesh());
	mSceneMgr->setAmbientLight(video::SColorf(1, 1, 1, 1));

	node->setPosition(core::vector3df(zoneModel->getX(), zoneModel->getY(), zoneModel->getZ()));

	//change later
	mSceneMgr->addCameraSceneNode();
}
