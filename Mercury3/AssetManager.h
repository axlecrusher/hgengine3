#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <mutex>

template<typename T>
class AssetManager {
public:
	typedef std::shared_ptr<T> AssetPtr;

	AssetManager() :is_valid(true) {}
	~AssetManager() { is_valid = false; }

	inline bool isValid() const { return is_valid; }

	inline AssetPtr get(const std::string& path) { return get(path, nullptr); }

	AssetPtr get(const std::string& path, bool* isNew) {
		AssetPtr asset = tryExisting(path);
		if (asset != nullptr) {
			if (isNew != nullptr) *isNew = false;
			return std::move(asset);
		}
		
		asset = AssetPtr(new T(), [this](T* asset) { this->release(asset); });
		bool success = asset->load(path);
		if (!success)
		{
			return AssetPtr(nullptr); //return null asset on fail
		}

		{
			//scoped block for mutex lock
			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			//check if some other thread created it while we were trying to
			auto ptr = find(path).lock();
			if (ptr != nullptr) {
				if (isNew != nullptr) *isNew = true;
				return std::move(ptr);
			}
			m_map.insert(std::make_pair(path, asset));
		}
		if (isNew != nullptr) *isNew = true;
		return std::move(asset);
	}

//	void remove(const std::string& path) { map.erase(path); }
private:
	inline AssetPtr tryExisting(const std::string& path) {
		std::weak_ptr<T> ptr;
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			ptr = find(path);
		}
		return std::move(ptr.lock());
	}

	inline std::weak_ptr<T> find(const std::string& path) {
		const auto& map = m_map;
		auto itr = map.find(path);
		if (itr == map.end()) return std::weak_ptr<T>();
		return itr->second;
	}

	void release(T* asset) {
		if (isValid()) { //make sure map hasn't been destroyed (when program exiting)
			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			m_map.erase(asset->m_path);
		}
		delete asset;
	}

	bool is_valid;
	std::unordered_map< std::string, std::weak_ptr<T> > m_map;
	std::recursive_mutex m_mutex;
};