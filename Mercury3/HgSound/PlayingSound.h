#pragma once

#include <HgSound/SoundAsset.h>

namespace HgSound {
	class PlayingSound {
	public:
		typedef std::shared_ptr<PlayingSound> ptr;

		PlayingSound(SoundAsset::ptr SoundAsset);

		void getSamples(uint32_t samples, float* buffer);
		bool isFinished() const { return m_nextSample >= m_sound->totalSamples(); }

		void stop();

	private:
		SoundAsset::ptr m_sound;
		uint64_t m_nextSample; //next sample to play
	};
}