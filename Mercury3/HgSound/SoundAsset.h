#pragma once

#include <AssetManager.h>

namespace HgSound {
	class PlayingSound;
	class Driver;

	class SoundAsset
	{
	public:
		typedef AssetManager<SoundAsset>::AssetPtr ptr;

		SoundAsset();

		static SoundAsset::ptr acquire(const std::string& path);

		float* data() const { return m_data.get(); }
		uint8_t channels() const { return m_channels; }
		uint64_t totalSamples() const { return m_totalSamples; }

		auto sampleRate() const { return m_sampleRate; }

	private:
		std::string m_path;
		std::unique_ptr<float[]> m_data;
		uint8_t m_channels;
		uint16_t m_sampleRate;
		uint64_t m_totalSamples; //all channels
		uint64_t m_sampleCount;

		std::weak_ptr<SoundAsset> selfPtr; //for creating shared pointers in play()

		bool load(const std::string& path);
		std::shared_ptr<PlayingSound> play();

		static AssetManager<SoundAsset> soundAssets;
		friend AssetManager<SoundAsset>;

		friend HgSound::Driver;
	};
}