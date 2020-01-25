#include "SoundAsset.h"

#define DR_WAV_IMPLEMENTATION
#include <HgSound/dr_wav.h>
#include <HgSound/PlayingSound.h>

namespace HgSound {

	AssetManager<SoundAsset> SoundAsset::soundAssets;

	BufferedWavSource::~BufferedWavSource()
	{
		delete[] m_samples;
	}

	StreamingWavSource::~StreamingWavSource()
	{
	}

	SamplePacket BufferedWavSource::getBuffer(IAudioSourceState& state) const
	{
		SamplePacket p;

		p.sampleCount = m_count;
		p.sampleBytes = 4;
		p.audioSamples = (char*)m_samples;
		p.hasMorePackets = false;

		return p;
	}

	std::unique_ptr<IAudioSourceState> BufferedWavSource::newState() const
	{
		return nullptr; //buffered sources don't have a state
	}

	std::unique_ptr<IAudioSourceState> StreamingWavSource::newState() const
	{
		auto state = std::make_unique<StreamingWavSource::State>();
		state->open(m_path.c_str());

		return std::move(state);
	}

	SamplePacket StreamingWavSource::getBuffer(IAudioSourceState& s) const
	{
		SamplePacket p;

		auto state = reinterpret_cast<StreamingWavSource::State*>(&s);
		drwav* wav = state->get_drwav();

		drwav_uint64 framesToRead = wav->sampleRate / 10; // 1/10th of a second
		uint32_t samplesCount = framesToRead * wav->channels;

		auto buffer = state->get_sampleBuffer();
		size_t framesDecoded = drwav_read_pcm_frames_f32(wav, framesToRead, buffer);

		p.sampleBytes = 4;
		p.sampleCount = framesDecoded * wav->channels;
		p.audioSamples = (char*)buffer;
		p.hasMorePackets = !(framesDecoded<framesToRead);


		return p;
	}

	StreamingWavSource::State::~State()
	{
		drwav_uninit(&m_wav);
		delete[] m_buffer;
	}

	void StreamingWavSource::State::open(const char* path)
	{
		auto wav = get_drwav();
		if (!drwav_init_file(wav, path, nullptr))
		{
			fprintf(stderr, "Could not open wave file \"%s\"\n", path);
		}

		drwav_uint64 framesToRead = wav->sampleRate / 10; // 1/10th of a second
		uint32_t samplesCount = framesToRead * wav->channels;

		m_buffer = new float[samplesCount];
	}

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

		const auto ps = std::make_shared<PlayingSound>(tmp, m_audioSource->newState());

		return ps;
	}
}