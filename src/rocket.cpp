
#include "rocket.h"
#include "renderer.h"

extern Renderer gRenderer;

void RocketRenderer::RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, 
	Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation)
{
	//we "compile" all of Rocket's geometry, so it never calls this
}

Rocket::Core::CompiledGeometryHandle RocketRenderer::CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, 
	int* indices, int num_indices, Rocket::Core::TextureHandle texture)
{
	RocketGeometry* geo = new RocketGeometry;
	geo->vertices = new video::S3DVertex[num_vertices];
	geo->indices = new uint16[num_indices];
	geo->num_vertices = num_vertices;
	geo->num_triangles = num_indices / 3;

	for (int i = 0; i < num_vertices; ++i)
	{
		auto& vert = vertices[i];
		new (&geo->vertices[i]) video::S3DVertex;
		video::S3DVertex& vertex = geo->vertices[i];
		vertex.Pos.X = vert.position.x;
		vertex.Pos.Y = vert.position.y;
		vertex.Color.set(vert.colour.alpha, vert.colour.red, vert.colour.green, vert.colour.blue);
		vertex.TCoords.X = vert.tex_coord.x;
		vertex.TCoords.Y = vert.tex_coord.y;
	}

	//need to correct winding order
	for (int i = 0; i < num_indices; i += 3)
	{
		geo->indices[i] = indices[i + 2];
		geo->indices[i + 1] = indices[i + 1];
		geo->indices[i + 2] = indices[i];
	}

	video::ITexture* tex = (video::ITexture*)texture;
	if (tex)
	{
		geo->material.setTexture(0, (video::ITexture*)texture);
		geo->material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
	}
	else
	{
		geo->material.MaterialType = video::EMT_TRANSPARENT_VERTEX_ALPHA;
	}

	return (Rocket::Core::CompiledGeometryHandle)geo;
}

void RocketRenderer::RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, 
	const Rocket::Core::Vector2f& translation)
{
	video::IVideoDriver* driver = gRenderer.getVideoDriver();
	RocketGeometry* geo = (RocketGeometry*)geometry;

	//translate vertices
	//irrlicht doesn't apply transforms on 2D draw calls, and I'm too lazy to do the whole 2D in 3D thing myself
	for (int i = 0; i < geo->num_vertices; ++i)
	{
		video::S3DVertex& vert = mVertexBuf[i];
		vert = geo->vertices[i];
		vert.Pos.X += translation.x;
		vert.Pos.Y += translation.y;
	}

	driver->setMaterial(geo->material);
	driver->draw2DVertexPrimitiveList(mVertexBuf, geo->num_vertices, geo->indices, geo->num_triangles);
}

void RocketRenderer::ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry)
{
	RocketGeometry* geo = (RocketGeometry*)geometry;
	delete[] geo->vertices;
	delete[] geo->indices;
	delete geo;
}

bool RocketRenderer::LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, 
	const Rocket::Core::String& source)
{
	bool isDDS = false;
	FileStream file(source.CString());
	video::ITexture* tex = gRenderer.createTexture(&file, source.CString(), isDDS);

	const core::dimension2du& dim = tex->getSize();
	texture_dimensions.x = dim.Width;
	texture_dimensions.y = dim.Height;
	texture_handle = (Rocket::Core::TextureHandle)tex;
	return true;
}

bool RocketRenderer::GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, 
	const Rocket::Core::Vector2i& dimensions)
{
	video::ITexture* tex = gRenderer.createTexture("Rocket", (void*)source, dimensions.x, dimensions.y, false);
	texture_handle = (Rocket::Core::TextureHandle)tex;
	return true;
}

void RocketRenderer::ReleaseTexture(Rocket::Core::TextureHandle texture)
{
	gRenderer.destroyTexture((video::ITexture*)texture);
}

void RocketRenderer::EnableScissorRegion(bool enable)
{

}

void RocketRenderer::SetScissorRegion(int x, int y, int width, int height)
{

}

float RocketRenderer::GetHorizontalTexelOffset()
{
	if (mIsDirectX)
		return 0.5f;
	return Rocket::Core::RenderInterface::GetHorizontalTexelOffset();
}

float RocketRenderer::GetVerticalTexelOffset()
{
	if (mIsDirectX)
		return 0.5f;
	return Rocket::Core::RenderInterface::GetVerticalTexelOffset();
}
