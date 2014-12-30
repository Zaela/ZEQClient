
#ifndef _ZEQ_ROCKET_H
#define _ZEQ_ROCKET_H

#include <rocket/Core.h>
#include <rocket/Controls.h>
#include <rocket/Core/Lua/Interpreter.h>
#include <rocket/Controls/Lua/Controls.h>
#include <irrlicht.h>

#include <chrono>
#include <vector>

#include "types.h"
#include "file_stream.h"

using namespace irr;

class RocketSystem : public Rocket::Core::SystemInterface
{
private:
	std::chrono::steady_clock::time_point mStartTime;

public:
	RocketSystem()
	{
		mStartTime = std::chrono::steady_clock::now();
	}

	virtual float GetElapsedTime() override
	{
		std::chrono::steady_clock::time_point cur = std::chrono::steady_clock::now();
		std::chrono::duration<float> dur = std::chrono::duration_cast<std::chrono::duration<float>>(cur - mStartTime);
		return dur.count();
	}

	virtual bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message) override
	{
		return true;
	}
};

class RocketRenderer : public Rocket::Core::RenderInterface
{
private:
	struct RocketGeometry
	{
		video::S3DVertex* vertices;
		uint16* indices;
		int num_vertices;
		int num_triangles;
		video::SMaterial material;
	};

	bool mIsDirectX;
	video::S3DVertex mVertexBuf[8192]; //may need to increase this or add special handling for large vertex batches

public:
	RocketRenderer() : mIsDirectX(false) { }

	void setIsDirectX(bool val) { mIsDirectX = val; }

	// Called by Rocket when it wants to render geometry that it does not wish to optimise.
	virtual void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, 
		Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation) override;

	// Called by Rocket when it wants to compile geometry it believes will be static for the forseeable future.
	virtual Rocket::Core::CompiledGeometryHandle CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, 
		int* indices, int num_indices, Rocket::Core::TextureHandle texture) override;

	// Called by Rocket when it wants to render application-compiled geometry.
	virtual void RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry,
		const Rocket::Core::Vector2f& translation) override;

	// Called by Rocket when it wants to release application-compiled geometry.
	virtual void ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry) override;

	// Called by Rocket when it wants to enable or disable scissoring to clip content.
	virtual void EnableScissorRegion(bool enable) override;

	// Called by Rocket when it wants to change the scissor region.
	virtual void SetScissorRegion(int x, int y, int width, int height) override;

	// Called by Rocket when a texture is required by the library.
	virtual bool LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, 
		const Rocket::Core::String& source) override;

	// Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
	virtual bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, 
		const Rocket::Core::Vector2i& source_dimensions) override;

	// Called by Rocket when a loaded texture is no longer required.
	virtual void ReleaseTexture(Rocket::Core::TextureHandle texture) override;

	/// Returns the native horizontal texel offset for the renderer.
	virtual float GetHorizontalTexelOffset() override;
	/// Returns the native vertical texel offset for the renderer.
	virtual float GetVerticalTexelOffset() override;
};

#endif
