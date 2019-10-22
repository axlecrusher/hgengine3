#pragma once

#include <string>
#include <memory>
#include <map>

#include "str_utils.h"
#include <AssetManager.h>

class HgTexture
{
public:
	enum format {
		UNKNOWN = 0,
		GRAY = 1,
		GRAY_ALPHA = 2,
		RGB = 3,
		RGBA = 4,
		DXT1 = 0x31545844,
		DXT2 = 0x32545844,
		DXT3 = 0x33545844,
		DXT4 = 0x34545844,
		DXT5 = 0x35545844, //DXT5
		BC5U_ATI = 0x32495441, //ATI2
		BC5U = 0x55354342, // BC5U
		BC5S = 0x53354342 //BC5S
};

	enum TextureType : uint8_t {
		DIFFUSE = 0,
		NORMAL = 1,
		SPECULAR = 2,
		TEXTURE_TYPE_COUNT
	};

	struct Properties
	{
		Properties()
			: width(0), height(0), format(HgTexture::format::UNKNOWN), mipMapCount(0)
		{}

		format format;
		uint16_t width, height;
		uint8_t mipMapCount;
	};

	~HgTexture();

	typedef void(*shared_ptr_delete)(HgTexture* t);

	/* TODO: I would like to make TexturePtr const since textures are immutible
	but current GPU state data is stored in the texture and needs to be
	updated after texture data is loaded. */
	typedef std::shared_ptr<HgTexture> TexturePtr;

	//used to obtain an image.
	static TexturePtr acquire(const std::string& path, TextureType type);

//	channels getChannels() const { return m_channels;  }

	inline TextureType getType() const { return m_type; }

	//Indicates that texture needs to be sent to gpu
	inline bool NeedsGPUUpdate() const { return needsUpdate; }

	inline size_t getUniqueId() const { return m_uniqueId; }

	void sendToGPU();
	inline uint32_t getGPUId() const { return gpuId; }

	Properties getProperties() const { return m_properties; }

	const unsigned char* getData() const { return data; }

	const std::string& getPath() const { return m_path; }

	using gpuUpdateTextureFunction = std::function<uint32_t(HgTexture*)>;
	static gpuUpdateTextureFunction updateTextureFunc;
private:
	//use HgTexture::acquire to instantiate a texture
	HgTexture();
	static void wireReload(HgTexture::TexturePtr ptr);

	bool load(const std::string& path);
	bool load_internal(std::string path);

	inline void setType(TextureType texType) { m_type = texType; }

	//Indicates that texture needs to be sent to gpu
	void setNeedsGPUUpdate(bool update) { needsUpdate = update; }

	//used when an image is no longer used. Called by TexturePtr, not user callible.
//	static void release(HgTexture* t);

	bool HgTexture::stb_load(FILE* f);
	bool HgTexture::dds_load(FILE* f);

	std::string m_path;
	unsigned char* data;

	TextureType m_type;
	Properties m_properties;

	uint32_t gpuId;
	bool needsUpdate;

	size_t m_uniqueId;

	static AssetManager<HgTexture> imageMap;
	friend AssetManager<HgTexture>;

	//friend uint32_t ogl_updateTextureData(HgTexture* tex);
};
