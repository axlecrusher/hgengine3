#pragma once

#include <stdint.h>
#include <core/Enumerations.h>

#include <HgVbo.h>
#include <HgGPUBuffer.h>
#include <Material.h>

#include <GLBuffer.h>
#include <EntityIdType.h>

class RenderData {
private:
	//We need to be able to support multiple VBOs without hardcoding more here.
	VboIndex m_vertexVbo;
	VboIndex m_indexVbo;
	VboIndex m_colorVbo;

public:
	typedef std::shared_ptr<RenderData> RenderDataPtr;

	static RenderDataPtr Create();
	static RenderDataPtr Create(const RenderDataPtr rdp);

	struct Flags {
		Flags() : BACKFACE_CULLING(true), DEPTH_WRITE(true)
		{}

		bool BACKFACE_CULLING : 1;
		bool DEPTH_WRITE : 1;
	};

	RenderData();
	~RenderData();

	//RenderData(const RenderData&) = delete;

	void render(const Instancing::InstancingMetaData* imd = nullptr);

	IHgVbo* hgVbo() { return m_vertexVbo.Record().Vbo().get(); }
	IHgVbo* indexVbo() { return m_indexVbo.Record().Vbo().get(); }
	IHgVbo* colorVbo() { return m_colorVbo.Record().Vbo().get(); }

	void VertexVboRecord(const VboIndex& vbo_index) { m_vertexVbo = vbo_index; }
	const HgVboRecord& VertexVboRecord() const { return m_vertexVbo.Record(); }

	void indexVboRecord(const VboIndex& vbo_index) { m_indexVbo = vbo_index; }
	const HgVboRecord& indexVboRecord() const { return m_indexVbo.Record(); }

	void colorVbo(const VboIndex& vbo_index) { m_colorVbo = vbo_index; }

	Material& getMaterial() { return m_material; }
	const Material& getMaterial() const { return m_material; }

	HgEngine::PrimitiveType getPrimitiveType() const { return m_primitive; }
	void setPrimitiveType(HgEngine::PrimitiveType t) { m_primitive = t; }

	//Copy per instance data into IGPUBuffers.
	//List should contain the entity IDs in the order they will be rendered.
	std::function<void(EntityIdType* list, uint32_t listLength)> CopyPerInstanceData;

	//void addPerInstanceAttribute()
	//std::vector<IGLBufferUse*> vertexAttributes;
	std::vector<IGPUBuffer*> perInstanceVertexAttributes;

	Flags renderFlags;

	//GLVaoId vao;
	VaoIndex vao;

private:
	Material m_material;
	HgEngine::PrimitiveType m_primitive;
};

typedef RenderData::RenderDataPtr RenderDataPtr;
//typedef std::shared_ptr<const RenderData> RenderDataPtr_const;