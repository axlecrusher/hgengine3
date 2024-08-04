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

	Material& operator=(const Material& rhs);

	void clearTextureIDs();

	void setShader(const HgShader& shader);
	HgShader& getShader() { return m_shader; }

	void updateGpuTextures();
	//bool updateTextures() const { return m_updateTextures; }

	//add texture as one that can be used by the material
	void addTexture(const HgTexture::TexturePtr& texture);
	void setUpdateTextures(bool t) { m_updateTextures = t; }

	bool isTransparent() const { return m_transparent; }
	void setTransparent(bool t) { m_transparent = t; m_recomputeHash = true;	}

	bool needsTexturesUpdated() const { return m_updateTextures; }

	HgTexture::Handle getGPUTextureHandle(HgTexture::TextureType t) const { return m_gpuTextureHandles[t]; }

	BlendMode blendMode() const { return m_blendMode; }
	void setBlendMode(BlendMode m) { m_blendMode = m; m_recomputeHash = true; }

	//size_t getUniqueId();

private:
	//void computeHash();

	//use the texture
	void setTexture(const HgTexture* t);

	HgShader m_shader;
	std::vector< HgTexture::TexturePtr > m_textures;
	BlendMode m_blendMode;
	HgTexture::Handle m_gpuTextureHandles[HgTexture::TEXTURE_TYPE_COUNT];

	bool m_transparent;
	bool m_updateTextures;
	bool m_recomputeHash;

	size_t m_uniqueId;
};