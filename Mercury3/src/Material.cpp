#include <Material.h>

void Material::clearTextureIDs() {
	memset(m_gpuTextureHandles, 0, sizeof(m_gpuTextureHandles));
}

void Material::setTexture(const HgTexture* t) {
	const HgTexture::TextureType type = t->getType();
	m_gpuTextureHandles[type] = t->getHandle();
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
	//setShader(HgShader::acquire("test_vertex.glsl", "test_frag.glsl"));
	setShader(HgShader::acquire("test_vertex_instanced.glsl", "test_frag_instanced.glsl"));
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
	m_recomputeHash = true;
}

void Material::ShaderDeleter::operator()(HgShader* shader)
{
	if (shader) HgShader::release(shader);
};

void Material::addTexture(const HgTexture::TexturePtr& texture) {
	m_textures.push_back(texture);
	m_updateTextures = true;
	m_recomputeHash = true;
}

//void Material::computeHash()
//{
//	size_t hash = m_shader->getUniqueId();
//	for (const auto& t : m_textures)
//	{
//		hash ^= t->getUniqueId();
//	}
//
//	std::hash<uint32_t> ih;
//	hash ^= ih(m_blendMode);
//
//	std::hash<bool> bh;
//	hash ^= bh(m_transparent);
//
//	m_uniqueId = hash;
//}
//
//size_t  Material::getUniqueId()
//{
//	if (m_recomputeHash)
//	{
//		computeHash();
//	}
//	m_recomputeHash = false;
//
//	return m_uniqueId;
//}