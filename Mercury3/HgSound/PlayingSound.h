#pragma once

#include <HgSound/SoundAsset.h>
#include <HgTimer.h>
#include <algorithm>

namespace HgSound {
	class PlayingSound {
	public:
		typedef std::shared_ptr<PlayingSound> ptr;
		typedef void(*playbackEndedFunc)(SoundAsset::ptr& p);

		PlayingSound(SoundAsset::ptr SoundAsset);

		void getSamples(uint32_t samples, float* buffer);
		bool isFinished() const { return m_nextSample >= m_sound->totalSamples(); }

		void stop();

		inline void setVolume(float x) { m_volume = x; }
		inline float volume() const { return m_volume; }

		//playbackEndedFunc callback will be called from a thread
		inline void setEventPlaybackEnded(const playbackEndedFunc& clbk) { m_playbackEndedClbk = clbk; }
		inline void eventPlaybackEnded() { if (m_playbackEndedClbk) m_playbackEndedClbk(m_sound); }

		void jumpToTime(HgTime t) {
			auto sound = m_sound.get();
			uint64_t msec = t.msec();
			auto s = msec * sound->sampleRate() * sound->channels();
			s /= 1000;
			m_nextSample = std::min(s, sound->totalSamples());
		}

		inline SoundAsset::ptr getSoundAsset() { return m_sound; }

	private:
		float m_volume;

		SoundAsset::ptr m_sound;
		uint64_t m_nextSample; //next sample to play
		playbackEndedFunc m_playbackEndedClbk;
	};
}