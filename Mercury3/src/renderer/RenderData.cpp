#include <RenderData.h>
#include <RenderBackend.h>

//RenderData::newRenderDataCallback RenderData::Create = nullptr;

RenderData::RenderData()
	:m_primitive(HgEngine::PrimitiveType::TRIANGLES)
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

RenderDataPtr RenderData::Create(const RenderDataPtr rdp)
{
	auto t = new RenderData(*rdp);
	std::shared_ptr<RenderData> rd(t);
	return rd;
}

void RenderData::render(const Instancing::InstancingMetaData* imd) {
	auto vertex = hgVbo();
	auto idx = indexVbo();
	auto color = colorVbo();

	if (vertex == nullptr) return;

	RENDERER()->setRenderAttributes(m_material.blendMode(), renderFlags);
	vertex->use();
	if (color != nullptr) color->use();

	if (imd)
	{
		if (idx)
		{
			idx->use();
			idx->draw(imd);
		}
		else
		{
			vertex->draw(imd);
		}
	}
	else
	{
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


}
