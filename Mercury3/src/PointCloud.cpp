#include "PointCloud.h"

namespace PointCloud
{

void PointCloud::update(HgTime tdelta)
{
}

void PointCloud::getInstanceData(gpuStruct* instanceData)
{
	const auto mat = getEntity().computeWorldSpaceMatrix();
	mat.store(instanceData->matrix);
}

void PointCloud::init() {
	IUpdatableInstance<gpuStruct>::init();

	HgEntity* e = &getEntity();
	RenderDataPtr rd = std::make_shared<RenderData>();
	auto shader = HgShader::acquire("tbn_visual_vert.glsl", "tbn_visual_frag.glsl");
	rd->getMaterial().setShader(shader);
	e->setRenderData(rd);
}

void PointCloud::buildFromVertexData(const vbo_layout_vnut* vboData, const uint32_t vertexCount)
{
	//we want to visualize the tangent, bitangent, and normal vectors.
	//These will be drawn as lines so. Each line needs 2 vertices (begin and end).
	//There will be 3 lines drawn so we need 6 vertices.
	m_points.resize(vertexCount);

	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		m_points[i].v = vboData[i].v;
		m_points[i].c = color8(0, 0, 255, 255);
	}

	const auto vbo = HgVbo::GenerateUniqueFrom(m_points.data(), m_points.size());
	getEntity().renderData()->VertexVboRecord(vbo);
	getEntity().renderData()->setPrimitiveType(HgEngine::PrimitiveType::POINTS);
}

} //PointCloud namespace