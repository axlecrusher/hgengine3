#include <RenderData.h>
#include <RenderBackend.h>

//RenderData::newRenderDataCallback RenderData::Create = nullptr;

RenderData::RenderData()
	:instanceCount(0)
	, gpuBuffer(nullptr), m_primitive(HgEngine::PrimitiveType::TRIANGLES)
{
}

RenderData::~RenderData()
{
}

RenderDataPtr RenderData::Create()
{
	auto t = new RenderData();
	std::shared_ptr<RenderData> rd(t);
	//auto rd = std::make_shared<RenderData>();
	return rd;
}

void RenderData::render() {
	auto vertex = hgVbo();
	auto idx = indexVbo();
	auto color = colorVbo();

	if (vertex == nullptr) return;

	RENDERER()->setRenderAttributes(m_material.blendMode(), renderFlags);
	vertex->use();
	if (color != nullptr) color->use();

	if (idx)
	{
		idx->use();
		idx->draw(this);
	}
	else
	{
		vertex->draw(this);
	}
}
