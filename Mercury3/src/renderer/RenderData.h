#pragma once

#include <stdint.h>

#include <HgVbo.h>
#include <HgGPUBuffer.h>
#include <Material.h>

//enum RenderFlags : uint8_t {
//	NONE = 0,
//	FACE_CULLING = 1,
//	DEPTH_WRITE = 2
//};


class RenderData {
private:
	//We need to be able to support multiple VBOs without hardcoding more here.
	VboIndex m_vertexVbo;
	VboIndex m_indexVbo;
	VboIndex m_colorVbo;

public:
	static std::shared_ptr<RenderData> Create() { return std::make_shared<RenderData>(); }

	struct Flags {
		Flags() : FACE_CULLING(true), DEPTH_WRITE(true)
		{}

		bool FACE_CULLING : 1;
		bool DEPTH_WRITE : 1;
	};

	RenderData();
	~RenderData();

	void render();

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

	uint32_t instanceCount;

	//std::shared_ptr<IHgGPUBuffer> gpuBuffer;
	IHgGPUBuffer* gpuBuffer;

	Flags renderFlags;

private:
	Material m_material;
};

typedef std::shared_ptr<RenderData> RenderDataPtr;
//typedef std::shared_ptr<const RenderData> RenderDataPtr_const;