#include <RenderData.h>
#include <RenderBackend.h>

//RenderData::newRenderDataCallback RenderData::Create = nullptr;

RenderData::RenderData()
	:instanceCount(0)
	, gpuBuffer(nullptr)
{
	init();
}

RenderData::~RenderData()
{
	destroy();
}

void RenderData::render() {
	RENDERER()->setRenderAttributes(m_material.blendMode(), renderFlags);
	hgVbo()->use();
	if (colorVbo() != nullptr) colorVbo()->use();
	indexVbo()->use();
	indexVbo()->draw(this);
}

void RenderData::destroy() {
	//FIXME: Do something to clean up hgVbo
	//hgvbo_remove(d->hgvbo, d->vbo_offset, d->vertex_count)
}

void RenderData::init() {
	//shader = HgShader::acquire("test_vertex.glsl", "test_frag.glsl");
}
