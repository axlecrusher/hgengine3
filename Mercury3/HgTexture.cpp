#include "HgTexture.h"
#include <glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>

HgTexture::gpu_update_texture HgTexture::updateTextureFunc = NULL;
HgTexture::mapContainer HgTexture::imageMap;

HgTexture::TexturePtr HgTexture::acquire(const std::string& path) {
	auto itr = imageMap.map.find(path);
	if (itr != imageMap.map.end()) {
		return itr->second.lock();
	}
	auto texture = TexturePtr(new HgTexture(), HgTexture::release);
	texture->load(path);
	imageMap.map.insert(std::make_pair(path, texture));
	return texture;
}

void HgTexture::release(HgTexture* t) {
	if (imageMap.is_valid) { //make sure map hasn't been destroyed (when program exiting)
		imageMap.map.erase(t->m_path);
	}
	delete t;
}

HgTexture::HgTexture()
{
	data = nullptr;
	m_width = m_height = 0;
	gpuId = 0;
	m_type = DIFFUSE;
	m_channels = HgTexture::channels::UNKNOWN;
}


HgTexture::~HgTexture()
{
	if (gpuId > 0) glDeleteTextures(1,&gpuId); //FIXME abstract this
	SAFE_FREE(data);
}


bool HgTexture::load(const std::string& path) {
	int x, y, fileChannels;
	m_path = path;
	stbi_set_flip_vertically_on_load(1);
	data = stbi_load(path.c_str(), &x, &y, &fileChannels, 0);
	m_channels = (HgTexture::channels)fileChannels;
	m_width = x;
	m_height = y;
	return data!=NULL;
}