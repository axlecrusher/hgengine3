#pragma once

#include <string>
#include <memory>
#include <map>
#include <atomic>

#include <AssetManager.h>

template<typename T>
class ThreadSafePtr
{
public:
	void set(std::shared_ptr<T>& p)
	{
		std::atomic_store(&m_ptr, p);
	}

	auto get() const
	{
		auto p = std::atomic_load(&m_ptr);
		return p;
	}
private:
	std::shared_ptr<T> m_ptr;
};

class HgTexture
{
public:
	typedef uint32_t Handle;
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
		Properties() noexcept
			: width(0), height(0), format(HgTexture::format::UNKNOWN), mipMapCount(0)
		{}

		format format;
		uint16_t width, height;
		uint8_t mipMapCount;
	};

	struct free_deleter {
		template <typename T>
		void operator()(T *p) const {
			free(p);
			p = NULL;
		}
	};

	//Struture to provide data from texture loading functions
	typedef struct LoadedTextureData
	{
		const unsigned char* getData() const { return m_data.get(); }
		std::unique_ptr<unsigned char, free_deleter> m_data;
		Properties properties;
	} LoadedTextureData;

	~HgTexture();

	HgTexture(const HgTexture& rhs) = delete;

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
	inline bool NeedsGPUUpdate() const { return m_needsUpdate; }

	//inline size_t getUniqueId() const { return m_uniqueId; }

	void sendToGPU();
	inline uint32_t getHandle() const { return m_handle; }

	Properties getProperties() const { return m_properties; }

	//const unsigned char* getData() const { return m_data.get(); }

	const std::string& getPath() const { return m_path; }

	//Thread safe
	void setLoadedTextureData(std::unique_ptr<LoadedTextureData>& ltd);
	//Thread safe
	std::shared_ptr<const LoadedTextureData> getLoadedTextureData() const;

	typedef struct GraphicsCallbacks
	{
		std::function<uint32_t(HgTexture*)> updateTexture;
		std::function<void(uint32_t)> deleteTexture;
	} GraphicsCallbacks;

	static GraphicsCallbacks gpuCallbacks;
private:

	//use HgTexture::acquire to instantiate a texture
	HgTexture();
	static void wireReload(HgTexture::TexturePtr ptr);

	bool load(const std::string& path);
	bool load_internal(std::string path);

	inline void setType(TextureType texType) { m_type = texType; }

	//Indicates that texture needs to be sent to gpu
	void setNeedsGPUUpdate(bool update) { m_needsUpdate = update; }

	//used when an image is no longer used. Called by TexturePtr, not user callible.
//	static void release(HgTexture* t);

	std::unique_ptr<LoadedTextureData> HgTexture::stb_load(FILE* f);
	std::unique_ptr<LoadedTextureData> HgTexture::dds_load(FILE* f);

	std::string m_path;
	TextureType m_type;
	Properties m_properties;
	Handle m_handle;
	std::atomic<bool> m_needsUpdate;

	ThreadSafePtr<const LoadedTextureData> m_loadedData;

	//size_t m_uniqueId;

	static AssetManager<HgTexture> imageMap;
	friend AssetManager<HgTexture>;

	//friend uint32_t ogl_updateTextureData(HgTexture* tex);
};
