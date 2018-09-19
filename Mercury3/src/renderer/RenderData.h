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

	IHgVbo* hgVbo() { return m_vertexVbo.VboRec().Vbo().get(); }
	IHgVbo* indexVbo() { return m_indexVbo.VboRec().Vbo().get(); }
	IHgVbo* colorVbo() { return m_colorVbo.VboRec().Vbo().get(); }

	void VertexVboRecord(const VboIndex& vbo_index) { m_vertexVbo = vbo_index; }
	const HgVboRecord& VertexVboRecord() const { return m_vertexVbo.VboRec(); }

	void indexVboRecord(const VboIndex& vbo_index) { m_indexVbo = vbo_index; }
	const HgVboRecord& indexVboRecord() const { return m_indexVbo.VboRec(); }

	void colorVbo(const VboIndex& vbo_index) { m_colorVbo = vbo_index; }
	//inline void colorVbo(std::unique_ptr<IHgVbo>& vbo) { m_colorVbo = std::move(vbo); }

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
	VboIndex m_vertexVbo;
	VboIndex m_indexVbo;
	VboIndex m_colorVbo;
};

typedef std::shared_ptr<RenderData> RenderDataPtr;