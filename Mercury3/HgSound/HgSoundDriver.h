#pragma once

#include <memory>
#include <vector>
#include <HgSound/SoundAsset.h>
#include <HgSound/PlayingSound.h>

#include <mutex>
#include <HgTimer.h>
#include <DoubleBuffer.h>
#include <thread>
#include <condition_variable>

namespace HgSound {
	class Driver {
	public:
		Driver();
		virtual ~Driver();
		virtual void Init() = 0;

		//start audio decoding and mixing thread
		virtual void start();

		PlayingSound::ptr play(SoundAsset::ptr& asset, HgTime startOffset);

		PlayingSound::ptr play(SoundAsset::ptr& asset) {
			const auto zero = HgTime::msec(0);
			return play(asset, zero);
		}

		void stop();

		void stopPlayback(PlayingSound::ptr& playingAsset);

		static std::unique_ptr<HgSound::Driver> Create();
		static void audioLoop(Driver* driver);

		void continueExecution();
	protected:
		const static int32_t samples;

		void mixAudio();

		DoubleBuffer<float> m_buffer;
		uint32_t m_bufferSize;

	private:
		void audioLoop();
		void wait();
		bool canContinue() const { return m_continueMixing;  }
		void InsertPlayingSound(PlayingSound::ptr& sound);
		PlayingSound::ptr RemovePlayingSound(PlayingSound::ptr& sound);

		bool m_stop;
		std::thread m_thread;

		std::map<const PlayingSound*, PlayingSound::ptr> m_playingSounds;
		std::recursive_mutex m_mutex;

		std::atomic<bool> m_continueMixing;
		std::mutex m_conditionMutex;
		std::condition_variable m_condition;
	};
}

extern std::unique_ptr<HgSound::Driver> SOUND;
