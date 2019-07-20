#include <RenderData.h>
#include <RenderBackend.h>

//RenderData::newRenderDataCallback RenderData::Create = nullptr;

RenderData::RenderData()
	:instanceCount(0)
	, gpuBuffer(nullptr)
{
}

RenderData::~RenderData()
{
}

void RenderData::render() {
	RENDERER()->setRenderAttributes(m_material.blendMode(), renderFlags);
	hgVbo()->use();
	if (colorVbo() != nullptr) colorVbo()->use();
	indexVbo()->use();
	indexVbo()->draw(this);
}
