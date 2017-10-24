#pragma once

#include <string>
#include <memory>
#include <map>

#include "str_utils.h"

class HgTexture
{
public:
	enum channels {
		UNKNOWN = 0,
		GRAY = 1,
		GRAY_ALPHA = 2,
		RGB = 3,
		RGBA = 4
	};

	enum TextureType {
		DIFFUSE = 0,
		NORMAL = 1,
		SPECULAR = 2,
		TEXTURE_TYPE_COUNT
	};

	HgTexture();
	~HgTexture();

	typedef void(*shared_ptr_delete)(HgTexture* t);
	typedef std::shared_ptr<HgTexture> TexturePtr;
	static TexturePtr acquire(const std::string& path);
	static void release(HgTexture* t);

	bool load(const std::string& path);
	channels getChannels() const { return m_channels;  }

	inline TextureType getType() const { return m_type; }
	inline void setType(TextureType texType) { m_type = texType; }

	bool getNeedsUpdate() const { return needsUpdate; }
	void setNeedsUpdate(bool update) { needsUpdate = update; }

	inline void sendToGPU() { gpuId = updateTextureFunc(m_width, m_height, m_channels, data); SAFE_FREE(data); }
	inline uint32_t getGPUId() const { return gpuId; }

	typedef uint32_t(*gpu_update_texture)(uint16_t x, uint16_t y, HgTexture::channels, unsigned char* data);
	static gpu_update_texture updateTextureFunc;
private:
	std::string m_path;
	unsigned char* data;
	channels m_channels;
	TextureType m_type;
	uint16_t m_width, m_height;
	uint32_t gpuId;
	bool needsUpdate;

	typedef struct mapContainer{
		mapContainer() :is_valid(true) {}
		~mapContainer() { is_valid=false; }
		bool is_valid;
		std::map< const std::string, std::weak_ptr<HgTexture> > map;

	} mapContainer;

	static mapContainer imageMap;
};

typedef int32_t(*gpu_update_texture)(uint16_t x, uint16_t y, HgTexture::channels);
	typedef std::shared_ptr<HgTexture> TexturePtr;
