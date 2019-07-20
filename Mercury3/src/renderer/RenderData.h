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

//enum RenderFlags : uint8_t {
//	NONE = 0,
//	FACE_CULLING = 1,
//	DEPTH_WRITE = 2
//};

class Material
{
public:
	Material();
	Material(const Material& other);

	void clearTextureIDs();
	void setTexture(const HgTexture* t);

	void setShader(HgShader* shader);
	HgShader& getShader() { return *m_shader.get(); }

	void updateGpuTextures();
	//bool updateTextures() const { return m_updateTextures; }

	void addTexture(const HgTexture::TexturePtr& texture);
	void setUpdateTextures(bool t) { m_updateTextures = t; }

	bool isTransparent() const { return m_transparent; }
	void setTransparent(bool t) { m_transparent = t; }

	bool needsTexturesUpdated() const { return m_updateTextures; }

	uint32_t getGPUTextureHandle(HgTexture::TextureType t) const { return m_gpuTextureHandles[t]; }

	BlendMode blendMode() const { return m_blendMode; }
	void setBlendMode(BlendMode m) { m_blendMode = m; }

private:
	struct ShaderReleaser {
		void operator()(HgShader* shader);
	};

	std::unique_ptr<HgShader, ShaderReleaser> m_shader;
	std::vector< HgTexture::TexturePtr > m_textures;
	BlendMode m_blendMode;
	uint32_t m_gpuTextureHandles[HgTexture::TEXTURE_TYPE_COUNT];

	bool m_transparent;
	bool m_updateTextures;
};

class RenderData {
private:
	//We need to be able to support multiple VBOs without hardcoding more here.
	VboIndex m_vertexVbo;
	VboIndex m_indexVbo;
	VboIndex m_colorVbo;

public:
	//typedef std::shared_ptr<RenderData>(*newRenderDataCallback)();
	//typedef void(*indiceRenderFunc)(RenderData* rd);
	//static newRenderDataCallback Create;
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

	void destroy();
	void init();

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

	//HgShader* shader;

	//RenderFlags renderFlags;

	//std::shared_ptr<IHgGPUBuffer> gpuBuffer;
	IHgGPUBuffer* gpuBuffer;

	//std::vector< HgTexture::TexturePtr > textures;

	Flags renderFlags;

private:
	Material m_material;
};

typedef std::shared_ptr<RenderData> RenderDataPtr;
//typedef std::shared_ptr<const RenderData> RenderDataPtr_const;