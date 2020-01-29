#include "WavSource.h"

#define DR_WAV_IMPLEMENTATION
#include <HgSound/dr_wav.h>

namespace HgSound
{

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

void BufferedWavSource::initializeState(std::unique_ptr<IAudioSourceState>& state) const
{
	state.reset(); //buffered sources don't have a state
}

void StreamingWavSource::initializeState(std::unique_ptr<IAudioSourceState>& state) const
{
	auto tmp = std::make_unique<StreamingWavSource::State>();
	tmp->open(m_path.c_str());

	state = std::move(tmp);
}

SamplePacket StreamingWavSource::getBuffer(IAudioSourceState& s) const
{
	SamplePacket p;

	auto& state = reinterpret_cast<StreamingWavSource::State&>(s);
	drwav* wav = state.get_drwav();

	drwav_uint64 framesToRead = wav->sampleRate / 10; // 1/10th of a second
	auto samplesCount = framesToRead * wav->channels;

	auto buffer = state.get_sampleBuffer();
	size_t framesDecoded = drwav_read_pcm_frames_f32(wav, framesToRead, buffer);

	p.sampleBytes = 4;
	p.sampleCount = framesDecoded * wav->channels;
	p.audioSamples = (char*)buffer;
	p.hasMorePackets = !(framesDecoded < framesToRead);


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
		return;
	}

	drwav_uint64 framesToRead = wav->sampleRate / 10; // 1/10th of a second
	auto samplesCount = framesToRead * wav->channels;

	m_buffer = new float[samplesCount];
}


}