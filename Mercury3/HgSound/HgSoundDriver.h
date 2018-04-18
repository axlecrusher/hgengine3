#pragma once

#include <memory>
#include <vector>
#include <HgSound/SoundAsset.h>
#include <HgSound/PlayingSound.h>

#include <mutex>

namespace HgSound {
	class Driver {
	public:
		Driver();
		virtual ~Driver();
		virtual void Init() = 0;

		virtual void start() = 0;

		PlayingSound::ptr play(SoundAsset::ptr asset);
		void stop(PlayingSound::ptr playingAsset);

		static std::unique_ptr<HgSound::Driver> Create();

	protected:
		static uint32_t samples;

		void mixAudio();

		float* m_buffer;
		uint32_t m_bufferSize;

		bool m_initialized;

	private:
		std::map<const PlayingSound*, PlayingSound::ptr> m_playingSounds;
		std::mutex m_mutex;
	};
}

extern std::unique_ptr<HgSound::Driver> SOUND;