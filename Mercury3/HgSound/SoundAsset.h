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

		std::shared_ptr<PlayingSound> newPlayingInstance() { return play(); }

		inline auto getSourceType() const { return m_audioSource->getSourceType(); }

		//returns true if an audio source is present
		inline bool isValid() const { return m_audioSource != nullptr; }

	private:
		std::string m_path;

		std::unique_ptr<IAudioSource> m_audioSource;

		std::weak_ptr<SoundAsset> selfPtr; //for creating shared pointers in play()

		bool load(const std::string& path);
		std::shared_ptr<PlayingSound> play();

		static AssetManager<SoundAsset> soundAssets;
		friend AssetManager<SoundAsset>;

		friend HgSound::Driver;
	};
}