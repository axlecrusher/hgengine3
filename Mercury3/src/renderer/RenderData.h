#pragma once

#include <stdint.h>
#include <core/Enumerations.h>

#include <HgVbo.h>
#include <HgGPUBuffer.h>
#include <Material.h>

#include <GLBuffer.h>

class RenderData {
private:
	//We need to be able to support multiple VBOs without hardcoding more here.
	VboIndex m_vertexVbo;
	VboIndex m_indexVbo;
	VboIndex m_colorVbo;

public:
	typedef std::shared_ptr<RenderData> RenderDataPtr;

	static RenderDataPtr Create();

	struct Flags {
		Flags() : BACKFACE_CULLING(true), DEPTH_WRITE(true)
		{}

		bool BACKFACE_CULLING : 1;
		bool DEPTH_WRITE : 1;
	};

	RenderData();
	~RenderData();

	void render(const Instancing::InstancingMetaData* imd = nullptr);

	IHgVbo* hgVbo() { return m_vertexVbo.VboRec().Vbo().get(); }
	IHgVbo* indexVbo() { return m_indexVbo.VboRec().Vbo().get(); }
	IHgVbo* colorVbo() { return m_colorVbo.VboRec().Vbo().get(); }

	void VertexVboRecord(const VboIndex& vbo_index) { m_vertexVbo = vbo_index; }
	const HgVboRecord& VertexVboRecord() const { return m_vertexVbo.VboRec(); }

	void indexVboRecord(const VboIndex& vbo_index) { m_indexVbo = vbo_index; }
	const HgVboRecord& indexVboRecord() const { return m_indexVbo.VboRec(); }

	void colorVbo(const VboIndex& vbo_index) { m_colorVbo = vbo_index; }
	//inline void colorVbo(std::unique_ptr<IHgVbo>& vbo) { m_colorVbo = std::move(vbo); }

	Material& getMaterial() { return m_material; }
	const Material& getMaterial() const { return m_material; }

	HgEngine::PrimitiveType getPrimitiveType() const { return m_primitive; }
	void setPrimitiveType(HgEngine::PrimitiveType t) { m_primitive = t; }

	//uint32_t instanceCount;

	////std::shared_ptr<IHgGPUBuffer> gpuBuffer;
	////IHgGPUBuffer* gpuBuffer;
	//std::shared_ptr<IHgGPUBuffer> gpuBuffer;

	Flags renderFlags;

	GLVaoId vao;

private:
	Material m_material;
	HgEngine::PrimitiveType m_primitive;
};

typedef RenderData::RenderDataPtr RenderDataPtr;
//typedef std::shared_ptr<const RenderData> RenderDataPtr_const;