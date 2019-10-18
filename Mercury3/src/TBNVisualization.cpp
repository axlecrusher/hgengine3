#include "TBNVisualization.h"

namespace TBNVisualization
{

void TBNVisualization::update(HgTime tdelta)
{
}

void TBNVisualization::getInstanceData(gpuStruct* instanceData)
{
	const auto mat = getEntity().computeWorldSpaceMatrix();
	mat.store(instanceData->matrix);
}

void TBNVisualization::init() {
	IUpdatableInstance<gpuStruct>::init();

	HgEntity* e = &getEntity();
	RenderDataPtr rd = std::make_shared<RenderData>();
	auto shader = HgShader::acquire("tbn_visual_vert.glsl", "tbn_visual_frag.glsl");
	rd->getMaterial().setShader(shader);
	e->setRenderData(rd);
}

vertex addVertexTangent(vertex v, const tangent& t)
{
	v.object.x(v.object.x() + t.x);
	v.object.y(v.object.y() + t.y);
	v.object.z(v.object.z() + t.z);

	return v;
}

vertex addVertexNormal(vertex v, const normal& n)
{
	v.object.x(v.object.x() + n.x);
	v.object.y(v.object.y() + n.y);
	v.object.z(v.object.z() + n.z);

	return v;
}

void TBNVisualization::buildFromVertexData(const vbo_layout_vnut* data, const uint32_t vertexCount)
{
	//we want to visualize the tangent, bitangent, and normal vectors.
	//These will be drawn as lines so. Each line needs 2 vertices (begin and end).
	//There will be 3 lines drawn so we need 6 vertices.
	m_lineVertices.resize(vertexCount * 6);

	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		auto* lineVertices = &m_lineVertices[i * 6];

		//tangent being
		lineVertices[0].v = data[i].v;
		lineVertices[0].c = color8(255, 0, 0, 255);

		//tangent end
		lineVertices[1].v = addVertexTangent(data[i].v, data[i].tan);
		lineVertices[1].c = color8(255, 0, 0, 255);

		//bitangent being
		lineVertices[2].v = data[i].v;
		lineVertices[2].c = color8(0, 255, 0, 255);

		//bitangent end
		lineVertices[3].v = data[i].v;
		lineVertices[3].c = color8(0, 255, 0, 255);

		//normal being
		lineVertices[4].v = data[i].v;
		lineVertices[4].c = color8(0, 0, 255, 255);

		//normal end
		lineVertices[5].v = addVertexNormal(data[i].v, data[i].n);
		lineVertices[5].c = color8(0, 0, 255, 255);
	}

	const auto vbo = HgVbo::GenerateUniqueFrom(m_lineVertices.data(), m_lineVertices.size());
	getEntity().renderData()->VertexVboRecord(vbo);
	getEntity().renderData()->setPrimitiveType(PrimitiveType::LINES);
}

} //TBNVisualization namespace