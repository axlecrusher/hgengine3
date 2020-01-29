#pragma once

#include <AssetManager.h>

#include "IAudioSource.h"

namespace HgSound {
	class PlayingSound;
	class Driver;

	class SoundAsset
	{
	public:

		typedef AssetManager<SoundAsset>::AssetPtr ptr;

		SoundAsset();

		static SoundAsset::ptr acquire(const std::string& path);

		//inline float* data() const { return m_audioSource->getBuffer(); }
		//inline SamplePacket getAudioData() const { return m_audioSource->getBuffer(); }
		inline IAudioSource& getAudioSource() const { return *m_audioSource; }

		uint8_t getNumChannels() const { return m_channels; }

		//total number of samples across all channels
		uint64_t totalSamples() const { return m_totalSamples; }

		//samples per second
		auto sampleRate() const { return m_sampleRate; }

		std::shared_ptr<PlayingSound> newPlayingInstance() { return play(); }

		inline auto getSourceType() const { return m_audioSource->sourceType(); }

		//returns true if an audio source is present
		inline bool isValid() const { return m_audioSource != nullptr; }

	private:
		std::string m_path;
		//std::unique_ptr<float[]> m_data;

		uint8_t m_channels;
		uint16_t m_sampleRate;
		uint64_t m_totalSamples; //all channels
		uint64_t m_frameCount;

		std::unique_ptr<IAudioSource> m_audioSource;

		std::weak_ptr<SoundAsset> selfPtr; //for creating shared pointers in play()

		bool load(const std::string& path);
		std::shared_ptr<PlayingSound> play();

		static AssetManager<SoundAsset> soundAssets;
		friend AssetManager<SoundAsset>;

		friend HgSound::Driver;
	};
}