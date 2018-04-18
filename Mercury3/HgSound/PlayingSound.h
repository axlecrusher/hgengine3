#pragma once

#include <HgSound/SoundAsset.h>

namespace HgSound {
	class PlayingSound {
	public:
		typedef std::shared_ptr<PlayingSound> ptr;
		typedef void(*playbackEndedFunc)(SoundAsset::ptr& p);

		PlayingSound(SoundAsset::ptr SoundAsset);

		void getSamples(uint32_t samples, float* buffer);
		bool isFinished() const { return m_nextSample >= m_sound->totalSamples(); }

		void stop();

		//playbackEndedFunc callback will be called from a thread
		inline void setEventPlaybackEnded(const playbackEndedFunc& clbk) { m_playbackEndedClbk = clbk; }
		inline void eventPlaybackEnded() { if (m_playbackEndedClbk) m_playbackEndedClbk(m_sound); }
	private:
		SoundAsset::ptr m_sound;
		uint64_t m_nextSample; //next sample to play
		playbackEndedFunc m_playbackEndedClbk;
	};
}