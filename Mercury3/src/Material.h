#pragma once

#include <memory>
#include <HgShader.h>
#include <HgTexture.h>
#include <vector>

enum BlendMode : uint8_t {
	BLEND_NORMAL = 0,
	BLEND_ADDITIVE,
	BLEND_ALPHA,
	BLEND_INVALID = 0xFF
};

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
	void setTransparent(bool t) { m_transparent = t; m_recomputeHash = true;	}

	bool needsTexturesUpdated() const { return m_updateTextures; }

	uint32_t getGPUTextureHandle(HgTexture::TextureType t) const { return m_gpuTextureHandles[t]; }

	BlendMode blendMode() const { return m_blendMode; }
	void setBlendMode(BlendMode m) { m_blendMode = m; m_recomputeHash = true; }

	size_t getUniqueId();

private:
	void computeHash();

	struct ShaderDeleter {
		void operator()(HgShader* shader);
	};

	std::unique_ptr<HgShader, ShaderDeleter> m_shader;
	std::vector< HgTexture::TexturePtr > m_textures;
	BlendMode m_blendMode;
	uint32_t m_gpuTextureHandles[HgTexture::TEXTURE_TYPE_COUNT];

	bool m_transparent;
	bool m_updateTextures;
	bool m_recomputeHash;

	size_t m_uniqueId;
};