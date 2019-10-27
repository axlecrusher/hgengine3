#include "HgTexture.h"
#include <FileWatch.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>

HgTexture::GraphicsCallbacks HgTexture::gpuCallbacks;

AssetManager<HgTexture> HgTexture::imageMap;

void HgTexture::wireReload(HgTexture::TexturePtr ptr)
{
	std::weak_ptr<HgTexture> wptr = ptr;
	WatchFileForChange(ptr->getPath(), [wptr]() {
		auto ptr = wptr.lock();
		ptr->load(ptr->getPath());
	});
}

HgTexture::TexturePtr HgTexture::acquire(const std::string& path, TextureType type) {
	bool isNew = false;
	auto ptr = imageMap.get(path, &isNew);
	if (ptr == nullptr) {
		fprintf(stderr, "Could not open image \"%s\"", path.c_str());
	}

	if (isNew) {
		ptr->setType(type);
		ptr->m_path = path;
		wireReload(ptr);
	}
	return std::move( ptr );
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
	m_handle = 0;
	m_type = DIFFUSE;
}


HgTexture::~HgTexture()
{
	if (getHandle() > 0) {
		//TODO: Fix gpuCallbacks being destroyed before textures
		if (gpuCallbacks.deleteTexture) gpuCallbacks.deleteTexture(getHandle());
	}

	m_handle = 0;
}

std::unique_ptr<HgTexture::LoadedTextureData> HgTexture::stb_load(FILE* f) {
	Properties p;
	int x, y, fileChannels;
//	stbi_set_flip_vertically_on_load(1);

	std::unique_ptr<unsigned char, free_deleter> data(stbi_load_from_file(f, &x, &y, &fileChannels, 0));
	bool success = (data != NULL);

	p.format = (HgTexture::format)fileChannels;
	p.width = x;
	p.height = y;
	fclose(f);

	if (success)
	{
		auto ltd = std::make_unique<LoadedTextureData>();
		ltd->m_data = std::move(data);
		ltd->properties = p;
		return std::move(ltd);
	}

	return nullptr;
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

typedef struct {
	uint32_t dxgiFormat;
	uint32_t resourceDimension;
	uint32_t miscFlag;
	uint32_t arraySize;
	uint32_t miscFlags2;
} DDS_HEADER_DXT10;

#define DDPF_FOURCC 0x4
#define DX10 0x30315844

std::unique_ptr<HgTexture::LoadedTextureData> HgTexture::dds_load(FILE* f) {
	bool success = false;
	DDS_HEADER header;
	DDS_HEADER_DXT10 dx10Header;

	if (fread(&header, 124, 1, f) != 1)
	{
		return false;
	}

	if (header.ddspf.flags == DDPF_FOURCC && header.ddspf.fourCC == DX10)
	{
		fread(&dx10Header, 20, 1, f);
	}

	Properties p;

	p.height = header.height;
	p.width = header.width;
	p.mipMapCount = header.mipMapCount;
	p.format = (HgTexture::format)header.ddspf.fourCC;

	const auto linearSize = header.pitchOrLinearSize;
	const uint32_t size = p.mipMapCount > 1 ? linearSize * 2 : linearSize;

	auto data = std::unique_ptr<unsigned char, free_deleter>((unsigned char*)malloc(size));

	const auto r = fread(data.get(), 1, size, f);
	//if (r == size)
	//{
		success = true;
	//}
	fclose(f);

	if (success)
	{
		auto ltd = std::make_unique<LoadedTextureData>();
		ltd->m_data = std::move(data);
		ltd->properties = p;
		return std::move(ltd);
		//ltd->path = std::move(path);
		//setLoadedTextureData(ltd);
	}

	return nullptr;
}

bool HgTexture::load(const std::string& path) {
	bool r = load_internal(path);
	if (r) {
		setNeedsGPUUpdate(true);
	}
	return r;
}

bool HgTexture::load_internal(std::string path) {
	//don't leave texture in an unknown state if load fails

	char filecode[4];
	FILE *f = fopen(path.c_str(), "rb");
	if (f == nullptr) {
		fprintf(stderr, "Unable to open file \"%s\"", path.c_str());
		return false;
	}

	std::unique_ptr<HgTexture::LoadedTextureData> ltd;

	fread(filecode, 1, 4, f);
	if (strncmp(filecode, "DDS ", 4) == 0)
	{
		ltd = dds_load(f);
	}
	else
	{
		fseek(f, 0, SEEK_SET);
		ltd = stb_load(f);
	}

	if (ltd!=nullptr)
	{
		//only update local data if texture loaded
		setLoadedTextureData(ltd);
		return true;
	}

	return false;
}

void HgTexture::sendToGPU()
{
//	gpuId = updateTextureFunc(m_width, m_height, m_channels, data);
	setNeedsGPUUpdate(false);
	auto ltd = getLoadedTextureData();
	m_properties = ltd->properties;
	m_handle = gpuCallbacks.updateTexture(this);
	ltd.reset();
}


void HgTexture::setLoadedTextureData(std::unique_ptr<LoadedTextureData>& ltd)
{
	std::shared_ptr<const LoadedTextureData> ptr(std::move(ltd));
	m_loadedData.set(ptr);
	setNeedsGPUUpdate(true);
}

std::shared_ptr<const HgTexture::LoadedTextureData> HgTexture::getLoadedTextureData() const
{
	return m_loadedData.get();
}
