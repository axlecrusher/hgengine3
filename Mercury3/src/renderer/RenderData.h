#pragma once

#include <stdint.h>
#include <memory>

#include <HgShader.h>
//#include <HgTexture.h>
#include <HgVbo.h>

#include <HgGPUBuffer.h>
#include <HgTexture.h>
#include <vector>

class HgShader;
//class IHgVbo;
class HgTexture;

enum BlendMode : uint8_t {
	BLEND_NORMAL = 0,
	BLEND_ADDITIVE,
	BLEND_ALPHA,
	BLEND_INVALID = 0xFF
};

enum RenderFlags : uint8_t {
	NONE = 0,
	FACE_CULLING = 1,
	DEPTH_WRITE = 2
};

class RenderData {
public:
	typedef std::shared_ptr<RenderData>(*newRenderDataCallback)();
	typedef void(*indiceRenderFunc)(RenderData* rd);
	static newRenderDataCallback Create;

	RenderData();
	virtual ~RenderData();

	inline void render() {
		renderFunction(this);
	}
	void destroy();
	void init();
	virtual void clearTextureIDs() = 0;
	virtual void setTexture(const HgTexture* t) = 0;

	HgShader* shader;

	indiceRenderFunc renderFunction; // could store VBO_TYPE instead and make a single function do all the rendering?

	inline IHgVbo* hgVbo() { return m_vertexVboRecord.Vbo().get(); }
	inline IHgVbo* indexVbo() { return m_indexVboRecord.Vbo().get(); }
	inline IHgVbo* colorVbo() { return m_colorVbo.get(); }

	inline void VertexVboRecord(HgVboRecord vbo_rec) { m_vertexVboRecord = vbo_rec; }
	inline const HgVboRecord& VertexVboRecord() const { return m_vertexVboRecord; }

	inline void indexVboRecord(HgVboRecord vbo_rec) { m_indexVboRecord = vbo_rec; }
	inline const HgVboRecord& indexVboRecord() const { return m_indexVboRecord; }

	inline void colorVbo(std::shared_ptr<IHgVbo>& vbo) { m_colorVbo = vbo; }
	inline void colorVbo(std::unique_ptr<IHgVbo>& vbo) { m_colorVbo = std::move(vbo); }

	void updateGpuTextures();

	BlendMode blendMode;
	RenderFlags renderFlags;

	//std::shared_ptr<IHgGPUBuffer> gpuBuffer;
	IHgGPUBuffer* gpuBuffer;

	uint32_t instanceCount;

	std::vector< HgTexture::TexturePtr > textures;
	bool updateTextures;

private:
	//We need to be able to support multiple VBOs without hardcoding more here.
	//std::shared_ptr<IHgVbo> m_hgVbo;
	HgVboRecord m_vertexVboRecord;
	HgVboRecord m_indexVboRecord;

	//std::shared_ptr<IHgVbo> m_indexVbo;
	std::shared_ptr<IHgVbo> m_colorVbo;
};

typedef std::shared_ptr<RenderData> RenderDataPtr;