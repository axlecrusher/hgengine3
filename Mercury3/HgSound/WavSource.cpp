#include "WavSource.h"

#define DR_WAV_IMPLEMENTATION
#include <HgSound/dr_wav.h>

#include <include/ThreadPool.h>

namespace HgSound
{

ThreadPool ioThreads(2);

BufferedWavSource::~BufferedWavSource()
{
	delete[] m_samples;
}

StreamingWavSource::~StreamingWavSource()
{
}

SamplePacket BufferedWavSource::getNextSamples(IAudioSourceState& state) const
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

SamplePacket Decode(StreamingWavSource::State* state)
{
	SamplePacket p;

	drwav* wav = state->get_drwav();

	drwav_uint64 framesToRead = wav->sampleRate / 10; // 1/10th of a second
	auto samplesCount = framesToRead * wav->channels;

	auto buffer = state->getBackBuffer();
	size_t framesDecoded = drwav_read_pcm_frames_f32(wav, framesToRead, buffer);
	state->swapBuffers();

	p.sampleBytes = 4;
	p.sampleCount = framesDecoded * wav->channels;
	p.audioSamples = (char*)buffer;
	p.hasMorePackets = !(framesDecoded < framesToRead);

	return p;
}

SamplePacket StreamingWavSource::getNextSamples(IAudioSourceState& s) const
{
	auto state = reinterpret_cast<StreamingWavSource::State*>(&s);

	SamplePacket r;

	//if the future is not ready replay the previous samples
	if (state->nextToPlay.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
	{
		return state->currentPlaying;
	}
	r = state->nextToPlay.get();
	state->currentPlaying = r;

	state->nextToPlay = ioThreads.enqueue([state] {
		return Decode(state);
	});

	return r;
}

StreamingWavSource::State::~State()
{
	if (nextToPlay.valid())
	{
		//if the future is valid, then wait for the IO thread to return. keep state valid.
		nextToPlay.wait();
	}

	drwav_uninit(&m_wav);

	if (m_frontBuffer) delete[] m_frontBuffer;
	if (m_backBuffer) delete[] m_backBuffer;
}

void StreamingWavSource::State::open(const char* path)
{
	auto wav = get_drwav();
	if (!drwav_init_file(wav, path, nullptr))
	{
		fprintf(stderr, "Could not open wave file \"%s\"\n", path);
		return;
	}

	m_info.channels = wav->channels;
	m_info.sampleRate = wav->sampleRate;

	drwav_uint64 framesToRead = wav->sampleRate / 10; // 1/10th of a second
	auto samplesCount = framesToRead * wav->channels;

	m_frontBuffer = new float[samplesCount]();
	m_backBuffer = new float[samplesCount]();

	auto state = this;
	nextToPlay = ioThreads.enqueue([state] {
		return Decode(state);
	});

	nextToPlay.wait();
}


}