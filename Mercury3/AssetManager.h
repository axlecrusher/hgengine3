#pragma once

#include <string>
#include <memory>
#include <map>

template<typename T>
class AssetManager {
public:
	typedef std::shared_ptr<T> AssetPtr;

	AssetManager() :is_valid(true) {}
	~AssetManager() { is_valid = false; }

	inline bool isValid() const { return is_valid; }

	AssetPtr get(const std::string& path) {
		auto itr = map.find(path);
		if (itr != map.end()) {
			auto t = itr->second.lock();
			if (t != nullptr) return std::move(t);
		}

		auto asset = AssetPtr(new T(), [this](T* asset) {this->release(asset); });
		asset->load(path);
		map.insert(std::make_pair(path, asset));
		return std::move(asset);
	}

//	void remove(const std::string& path) { map.erase(path); }
private:
	void release(T* asset) {
		if (isValid()) { //make sure map hasn't been destroyed (when program exiting)
			map.erase(asset->m_path);
		}
		delete asset;
	}

	bool is_valid;
	std::map< const std::string, std::weak_ptr<T> > map;
};