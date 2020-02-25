#include "SoundAsset.h"

#include <HgSound/PlayingSound.h>

#include <WavSource.h>
#include <OggSource.h>

namespace HgSound {

	AssetManager<SoundAsset> SoundAsset::soundAssets;

	std::string SoundFileLoader::uniqueIdentifier() const
	{
		return m_path;
	}

	bool SoundFileLoader::load(AssetPtr& asset) const
	{
		return asset->load(m_path);
	}

	SoundAsset::SoundAsset()
	{
	}

	SoundAsset::ptr SoundAsset::acquire(const std::string& path) {
		SoundFileLoader loader(path);

		bool isNew = false;
		ptr tmp = soundAssets.get(loader, &isNew);
		tmp->selfPtr = tmp;
		return std::move(tmp);
	}

	bool SoundAsset::load(const std::string& path) {
		unsigned int channels, sampleRate;

		m_path = path;

		
		std::unique_ptr<IAudioSource> source;

		auto r = path.rfind(".ogg");
		if (r != std::string::npos)
		{
			source = std::make_unique<StreamingOggSource>(path);
		}
		else if (path.rfind(".wav") != std::string::npos)
		{
			drwav wav;

			if (!drwav_init_file(&wav, path.c_str(), nullptr))
			{
				return false;
			}

			double duration = wav.totalPCMFrameCount;
			duration /= wav.sampleRate;
			if (duration < 10.0)
			{
				size_t samplesCount = wav.totalPCMFrameCount * wav.channels;
				auto buffer = new float[samplesCount];

				size_t framesDecoded = drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, buffer);
				source = std::make_unique<BufferedWavSource>(buffer, (uint32_t)samplesCount);

			}
			else
			{
				source = std::make_unique<StreamingWavSource>(path);
			}

			drwav_uninit(&wav);
		}
		m_audioSource = std::move(source);

		return true;
	}

	PlayingSound::ptr SoundAsset::play() {
		ptr tmp = selfPtr.lock();

		if (tmp == nullptr) return nullptr;

		std::unique_ptr<IAudioSourceState> state;
		m_audioSource->initializeState(state);

		const auto ps = std::make_shared<PlayingSound>(tmp, state);

		return ps;
	}
}