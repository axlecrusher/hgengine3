#pragma once

#include <memory>
#include <vector>
#include <HgSound/SoundAsset.h>
#include <HgSound/PlayingSound.h>

#include <thread>
#include <mutex>

namespace HgSound {
	class Driver {
	public:
		Driver();
		virtual ~Driver();
		virtual void init() = 0;

		void start();

		PlayingSound::ptr play(SoundAsset::ptr asset);
		void stop(PlayingSound::ptr playingAsset);

		static std::unique_ptr<HgSound::Driver> Create();

	protected:
		static uint32_t samples;

		void mixAudio();

		float* m_buffer;

		uint32_t m_bufferSize;

		bool m_initialized;

		inline void ReadyMix() {
			std::unique_lock<std::mutex> tmp(m_mixMutex);
			m_mixWait.notify_all();
		}
	private:
		static void threadLoop(Driver* driver);
		inline void stopThread() {
			m_stopThread = true;
			ReadyMix(); //signal thread to proceed
			m_mixThread.join();
		}

		std::map<const PlayingSound*, PlayingSound::ptr> m_playingSounds;
		std::mutex m_mutex;

		bool m_stopThread;
		std::thread m_mixThread;
		std::condition_variable m_mixWait;
		std::mutex m_mixMutex;
	};
}

extern std::unique_ptr<HgSound::Driver> SOUND;