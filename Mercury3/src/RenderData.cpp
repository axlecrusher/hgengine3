#include <RenderData.h>

RenderData::newRenderDataCallback RenderData::Create = nullptr;

RenderData::RenderData()
	:blendMode(BlendMode::BLEND_NORMAL), renderFunction(nullptr),
	vbo_offset(0), vertex_count(0), index_count(0),
	renderFlags(RenderFlags(FACE_CULLING | DEPTH_WRITE))
{

}

RenderData::~RenderData()
{
	destroy();
}

void RenderData::destroy() {
	//FIXME: Do something to clean up hgVbo
	//hgvbo_remove(d->hgvbo, d->vbo_offset, d->vertex_count)

	if (shader) HgShader::release(shader);
	shader = nullptr;
}

void RenderData::init() {
	shader = HgShader::acquire("test_vertex.glsl", "test_frag.glsl");
}
