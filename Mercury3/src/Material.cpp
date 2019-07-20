#include <Material.h>

void Material::clearTextureIDs() {
	memset(m_gpuTextureHandles, 0, sizeof(m_gpuTextureHandles));
}

void Material::setTexture(const HgTexture* t) {
	const HgTexture::TextureType type = t->getType();
	m_gpuTextureHandles[type] = t->getGPUId();
}

void Material::updateGpuTextures() {
	clearTextureIDs();

	for (auto& texture : m_textures)
	{
		if (texture->NeedsGPUUpdate()) {
			texture->sendToGPU();
			HgTexture::TextureType type = texture->getType();
		}

		setTexture(texture.get());
	}
}

Material::Material()
	:m_updateTextures(false), m_transparent(false), m_blendMode(BlendMode::BLEND_NORMAL)
{
	clearTextureIDs();
	setShader(HgShader::acquire("test_vertex.glsl", "test_frag.glsl"));
}

Material::Material(const Material& other)
	: m_textures(other.m_textures), m_transparent(other.m_transparent),
	m_blendMode(other.m_blendMode), m_updateTextures(other.m_updateTextures)
{
	memcpy(m_gpuTextureHandles, other.m_gpuTextureHandles, sizeof(m_gpuTextureHandles));
	setShader(HgShader::acquire(other.m_shader.get()));
}

void Material::setShader(HgShader* shader)
{
	m_shader = std::unique_ptr<HgShader, ShaderDeleter>(shader);
}

void Material::ShaderDeleter::operator()(HgShader* shader)
{
	if (shader) HgShader::release(shader);
};

void Material::addTexture(const HgTexture::TexturePtr& texture) {
	m_textures.push_back(texture);
	m_updateTextures = true;
}