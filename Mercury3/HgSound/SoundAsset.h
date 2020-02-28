#pragma once

#include <AssetManager.h>

#include "IAudioSource.h"

namespace HgSound {
	class PlayingSound;
	class Driver;

	class SoundFileLoader;

	class SoundAsset
	{
	public:
		using AssetManagerType = AssetManager<SoundAsset>;
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

		friend SoundFileLoader;

		friend HgSound::Driver;
	};

	class SoundFileLoader : public SoundAsset::AssetManagerType::IAssetLoader
	{
		using AssetPtr = SoundAsset::AssetManagerType::IAssetLoader::AssetPtr;
	public:
		SoundFileLoader(const std::string& path)
			:m_path(path)
		{}
		virtual std::string uniqueIdentifier() const;
		virtual bool load(AssetPtr& asset) const;

		virtual bool sourceChanged() const { return false; }

	private:
		std::string m_path;
	};

}