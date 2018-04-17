#include <HgSound/PlayingSound.h>

namespace HgSound {

	PlayingSound::PlayingSound(SoundAsset::ptr asset)
		: m_sound(std::move(asset)), m_nextSample(0), m_playbackEndedClbk(nullptr)
	{

	}

	void PlayingSound::getSamples(uint32_t samples, float* buffer) {
		//samples are unformatted sample count. samples/channels = whole samples
//		uint8_t channels = m_sound->channels();
		float* data = m_sound->data() + m_nextSample;
		auto totalSamples = m_sound->totalSamples();
		uint32_t i = 0;

		while ((i<samples) && (m_nextSample<totalSamples)) {
			buffer[i] += data[i];
			i++;
			m_nextSample++;
		}
	}

	void PlayingSound::stop() {
	}
}