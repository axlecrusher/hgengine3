#include "HgTexture.h"
#include <glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>

HgTexture::gpu_update_texture HgTexture::updateTextureFunc = NULL;
AssetManager<HgTexture> HgTexture::imageMap;

HgTexture::TexturePtr HgTexture::acquire(const std::string& path) {
	return imageMap.get(path);
}
/*
void HgTexture::release(HgTexture* t) {
	if (imageMap.isValid()) { //make sure map hasn't been destroyed (when program exiting)
		imageMap.remove(t->m_path);
	}
	delete t;
}
*/
HgTexture::HgTexture()
{
	data = nullptr;
	m_width = m_height = 0;
	gpuId = 0;
	m_type = DIFFUSE;
	m_format = HgTexture::format::UNKNOWN;
	m_mipMapCount = 0;
	m_linearSize = 0;
}


HgTexture::~HgTexture()
{
	if (gpuId > 0) glDeleteTextures(1,&gpuId); //FIXME abstract this
	SAFE_FREE(data);
}

bool HgTexture::stb_load(FILE* f) {
	int x, y, fileChannels;
//	stbi_set_flip_vertically_on_load(1);
	data = stbi_load_from_file(f, &x, &y, &fileChannels, 0);
	m_format = (HgTexture::format)fileChannels;
	m_width = x;
	m_height = y;
	fclose(f);
	return data != NULL;
}

struct DDS_PIXELFORMAT {
	uint32_t size;
	uint32_t flags;
	uint32_t fourCC;
	uint32_t RGBBitCount;
	uint32_t RBitMask;
	uint32_t GBitMask;
	uint32_t BBitMask;
	uint32_t ABitMask;
};

typedef struct {
	uint32_t size;
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitchOrLinearSize;
	uint32_t depth;
	uint32_t mipMapCount;
	uint32_t reserved[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t caps;
	uint32_t caps2;
	uint32_t caps3;
	uint32_t caps4;
	uint32_t reserved2;
} DDS_HEADER;

bool HgTexture::dds_load(FILE* f) {
	DDS_HEADER header;

	fread(&header, 124, 1, f);
	m_height = header.height;
	m_width = header.width;
	m_mipMapCount = header.mipMapCount;
	m_linearSize = header.pitchOrLinearSize;
	m_format = (HgTexture::format)header.ddspf.fourCC;

	uint32_t size = m_mipMapCount > 1 ? m_linearSize * 2 : m_linearSize;

	data = (unsigned char*)malloc(size);
	fread(data, 1, size, f);
	fclose(f);

	return true;
}

bool HgTexture::load(const std::string& path) {
	char filecode[4];
	m_path = path;
	FILE *f = fopen(path.c_str(), "rb");

	fread(filecode, 1, 4, f);
	if (strncmp(filecode, "DDS ", 4) != 0) {
		fseek(f, 0, SEEK_SET);
		return stb_load(f);
	}

	return dds_load(f);
}

void HgTexture::sendToGPU()
{
//	gpuId = updateTextureFunc(m_width, m_height, m_channels, data);
	gpuId = updateTextureFunc(this);
	SAFE_FREE(data);
}
