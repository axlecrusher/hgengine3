#include "SoundAsset.h"

#include <HgSound/PlayingSound.h>

#include <WavSource.h>

namespace HgSound {

	AssetManager<SoundAsset> SoundAsset::soundAssets;

	SoundAsset::SoundAsset() : m_channels(0), m_sampleRate(0),
		m_totalSamples(0), m_frameCount(0)
	{
	}

	SoundAsset::ptr SoundAsset::acquire(const std::string& path) {
		ptr tmp = soundAssets.get(path);
		tmp->selfPtr = tmp;
		return std::move(tmp);
	}

	bool SoundAsset::load(const std::string& path) {
		unsigned int channels, sampleRate;
		drwav_uint64 totalSampleCount;

		m_path = path;

		drwav wav;

		if (!drwav_init_file(&wav, path.c_str(), nullptr))
		{
			return false;
		}

		std::unique_ptr<IAudioSource> source;

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

		m_sampleRate = wav.sampleRate;
		m_channels = wav.channels;
		m_totalSamples = wav.totalPCMFrameCount * wav.channels; //includes all channels
		m_frameCount = wav.totalPCMFrameCount;

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