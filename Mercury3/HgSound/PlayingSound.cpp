#include <HgSound/PlayingSound.h>
#include <HgSoundDriver.h>

namespace HgSound {

	PlayingSound::PlayingSound(SoundAsset::ptr& asset, std::unique_ptr<IAudioSourceState>& sourceState)
		: m_sound(asset), m_playbackEndedClbk(nullptr), m_volume(1.0), m_audioSourceState(std::move(sourceState))
	{
		if (!m_audioSourceState)
		{
			fprintf(stderr, "Audio source state null\n");
		}
	}

//	void PlayingSound::getSamples(uint32_t samples, float* buffer) {
//		//samples are unformatted sample count. samples/channels = whole samples
////		uint8_t channels = m_sound->channels();
//		float* data = m_sound->data() + m_nextSample;
//		auto totalSamples = m_sound->totalSamples();
//		uint32_t i = 0;
//
//		while ((i<samples) && (m_nextSample<totalSamples)) {
//			buffer[i] += data[i] * m_volume;
//			i++;
//			m_nextSample++;
//		}
//	}

	void PlayingSound::stop() {
		SOUND->stopPlayback(this);
	}

	Emitter PlayingSound::EmitFromEntity(const HgEntity* e)
	{
		m_emittingEntityId = e->getEntityId();

		HgSound::Emitter emitter;
		emitter.setPosition(e->position());
		setEmitter(emitter);
		return emitter;
	}

}