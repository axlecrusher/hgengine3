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
	auto shader = HgShader::acquire("assets/shaders/tbn_visual.vert", "assets/shaders/tbn_visual.glsl");
	rd->getMaterial().setShader(shader);
	e->setRenderData(rd);
}

void TBNVisualization::buildFromVertexData(const HgVboRecord& vborec)
{
	const auto vertexCount = vborec.Count();
	switch (vborec.Vbo()->VboType())
	{
	case VBO_VNUT:
		{
			auto data = vborec.getBufferAs<vbo_layout_vnut>();
			buildFromVertexData(data, vertexCount);
		}
		break;
	//case VBO_VNU:
	//	{
	//		auto data = vborec.getBufferAs<vbo_layout_vnu>();
	//		//buildFromVertexData(data, vertexCount);
	//	}
	break;
	}
}

void TBNVisualization::buildFromVertexData(const vbo_layout_vnut* vboData, const uint32_t vertexCount)
{
	//we want to visualize the tangent, bitangent, and normal vectors.
	//These will be drawn as lines so. Each line needs 2 vertices (begin and end).
	//There will be 3 lines drawn so we need 6 vertices.
	m_lineVertices.resize(vertexCount * 6);

	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		const auto& data = vboData[i];
		auto* lineVertices = &m_lineVertices[i * 6];

		const auto& norm = data.n;
		const auto& tan = data.tan;

		const vector3f normal(norm.x, norm.y, norm.z);
		const vector3f tangent(tan.x, tan.y, tan.z);
		const auto bt = normal.cross(tangent).scale(tan.w);

		//tangent being
		lineVertices[0].v = data.v;
		lineVertices[0].c = color8(255, 0, 0, 255);

		//tangent end
		lineVertices[1].v.object = data.v.object + tangent;
		lineVertices[1].c = color8(255, 0, 0, 255);

		//bitangent being
		lineVertices[2].v = data.v;
		lineVertices[2].c = color8(0, 255, 0, 255);

		//bitangent end
		lineVertices[3].v.object = data.v.object + bt;
		lineVertices[3].c = color8(0, 255, 0, 255);

		//normal being
		lineVertices[4].v = data.v;
		lineVertices[4].c = color8(0, 0, 255, 255);

		//normal end
		lineVertices[5].v.object = data.v.object + normal;
		lineVertices[5].c = color8(0, 0, 255, 255);
	}

	const auto vbo = HgVbo::GenerateUniqueFrom(m_lineVertices.data(), m_lineVertices.size());
	getEntity().getRenderDataPtr()->VertexVboRecord(vbo);
	getEntity().getRenderDataPtr()->setPrimitiveType(HgEngine::PrimitiveType::LINES);
}

} //TBNVisualization namespace