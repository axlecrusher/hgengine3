#include "OggSource.h"
#include <include/ThreadPool.h>
#include <algorithm>
#include <Logging.h>

namespace HgSound
{

static ThreadPool ioThreads(2);

//bool StreamingOggSource::isOgg(char* path)
//{
//	return false;
//}

StreamingOggSource::~StreamingOggSource()
{
}

void StreamingOggSource::initializeState(std::unique_ptr<IAudioSourceState>& state) const
{
	auto tmp = std::make_unique<StreamingOggSource::State>();
	tmp->open(m_path.c_str());

	state = std::move(tmp);
}

SamplePacket StreamingOggSource::State::Decode(StreamingOggSource::State* state)
{
	SamplePacket p;

	auto info = ov_info(state->get_vorbis(), -1);

	int bytesToRead = sizeof(uint16_t) * state->m_samplesToRead;
	int bitstream;

	int bytesRead = 0;

	int totalSamplesRead = 0;
	while (bytesRead < bytesToRead)
	{
		int16_t buffer[2096];
		int toRead = std::min(4096, bytesToRead - bytesRead);
		auto r = ov_read(state->get_vorbis(), (char*)&buffer, toRead, 0, 2, 1, &bitstream);

		if (r == 0)
		{
			p.hasMorePackets = false;
			return p; //nothing to read
		}

		bytesRead += r;

		const auto samplesRead = r / 2;
		auto backBuffer = state->getBackBuffer();
		for (int i = 0; i < samplesRead; i++)
		{
			backBuffer[totalSamplesRead] = buffer[i] / 32768.0;
			totalSamplesRead++;
		}

		p.sampleBytes = 4;
		p.sampleCount = totalSamplesRead;
		p.audioSamples = (char*)backBuffer;
		p.hasMorePackets = true;
	}

	return p;
}

SamplePacket StreamingOggSource::getNextSamples(IAudioSourceState& s) const
{
	auto state = reinterpret_cast<StreamingOggSource::State*>(&s);

	SamplePacket r;

	//if the future is not ready replay the previous samples
	if (state->nextToPlay.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
	{
		return state->currentPlaying;
	}
	r = state->nextToPlay.get();
	state->currentPlaying = r;

	state->nextToPlay = ioThreads.enqueue([state] {
		return state->Decode(state);
	});

	return r;
}

StreamingOggSource::State::~State()
{
	if (nextToPlay.valid())
	{
		//if the future is valid, then wait for the IO thread to return. keep state valid.
		nextToPlay.wait();
	}

	if (m_vorbisPtr)
	{
		ov_clear(m_vorbisPtr.get());
	}
	m_vorbisPtr = nullptr;

	if (m_frontBuffer) delete[] m_frontBuffer;
	if (m_backBuffer) delete[] m_backBuffer;
}

void StreamingOggSource::State::open(const char* path)
{
	auto ptr = std::make_unique<OggVorbis_File>();
	auto r = ov_fopen(path, ptr.get()); //FIXME: this should be done in a thread

	if (r < 0)
	{
		LOG_ERROR("Could not open vorbis file \"%s\"", path);
		return;
	}

	m_vorbisPtr = std::move(ptr);

	auto info = ov_info(m_vorbisPtr.get(), -1);

	m_info.channels = info->channels;
	m_info.sampleRate = info->rate;

	auto framesToRead = info->rate / 10; // 1/10th of a second
	m_samplesToRead = framesToRead * info->channels;

	m_frontBuffer = new float[m_samplesToRead]();
	m_backBuffer = new float[m_samplesToRead]();

	auto state = this;
	nextToPlay = ioThreads.enqueue([state] {
		return Decode(state);
	});

	nextToPlay.wait();
}

}