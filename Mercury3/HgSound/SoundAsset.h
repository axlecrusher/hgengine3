#pragma once

#include <AssetManager.h>

namespace HgSound {
	class PlayingSound;

	class SoundAsset
	{
	public:
		typedef AssetManager<SoundAsset>::AssetPtr ptr;

		SoundAsset();
		~SoundAsset();

		static SoundAsset::ptr acquire(const std::string& path);

		std::shared_ptr<PlayingSound> play();

		float* data() const { return m_data; }
		uint8_t channels() const { return m_channels; }
		uint64_t totalSamples() const { return m_totalSamples; }

	private:
		std::string m_path;
		float* m_data;
		uint8_t m_channels;
		uint16_t m_sampleRate;
		uint64_t m_totalSamples; //all channels
		uint64_t m_sampleCount;

		std::weak_ptr<SoundAsset> selfPtr; //for creating shared pointers in play()

		bool load(const std::string& path);

		static AssetManager<SoundAsset> soundAssets;
		friend AssetManager<SoundAsset>;
	};
}