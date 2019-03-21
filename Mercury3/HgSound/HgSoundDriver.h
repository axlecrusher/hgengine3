#pragma once

#include <memory>
#include <vector>
#include <HgSound/SoundAsset.h>
#include <HgSound/PlayingSound.h>

#include <mutex>
#include <HgTimer.h>
#include <DoubleBuffer.h>
#include <thread>
#include <ConditionalWait.h>

namespace HgSound {
	class Driver {
	public:
		Driver();
		virtual ~Driver();
		virtual bool Init() = 0;

		//start audio decoding and mixing thread
		virtual bool start();

		virtual void threadLoop() = 0;

		virtual void play(PlayingSound::ptr& sound, HgTime startOffset);

		void play(PlayingSound::ptr& sound) {
			const auto zero = HgTime::msec(0);
			return play(sound, zero);
		}

		void stop();

		void stopPlayback(PlayingSound::ptr& playingAsset);

		static std::unique_ptr<HgSound::Driver> Create();
		void mixingLoop();

		void continueExecution() { m_wait.resume(); }
	protected:
		const static int32_t samples;

		bool stopping() { return m_stop; }
		void mixAudio();

		DoubleBuffer<float> m_buffer;
		void InsertPlayingSound(PlayingSound::ptr& sound);

	private:
		auto PlayingSounds();
		void audioLoop();
		void wait() { m_wait.wait(); }
		PlayingSound::ptr RemovePlayingSound(PlayingSound::ptr& sound);

		bool m_stop;
		std::thread m_thread;

		typedef std::vector<PlayingSound::ptr> PlayingSoundList;
		std::vector< PlayingSound::ptr> m_playingSounds;
		//std::vector< PlayingSound::ptr> m_tmpSounds; //accumulates still playing sounds
		std::recursive_mutex m_mutex;

		ConditionalWait m_wait;
	};

	template<typename T>
	void atomic_swap(T &x, T& y, std::recursive_mutex& mutex) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		std::swap(x, y);
	}

	template<typename T>
	void atomic_concat(std::vector<T> &src, std::vector<T>& dest, std::recursive_mutex& mutex) {
		std::lock_guard<std::recursive_mutex> lock(mutex);

		if (dest.size() == 0) {
			std::swap(src, dest);
			return;
		}

		//copy smaller array into larger array
		if (dest.size() < src.size()) {
			//if destination smaller, copy it into src and swap
			src.reserve(src.size() + dest.size());
			src.insert(src.end(), dest.begin(), dest.end());
			std::swap(dest, src);
		}
		else {
			dest.reserve(src.size() + dest.size());
			dest.insert(dest.end(), src.begin(), src.end());
		}
	}
}

extern std::unique_ptr<HgSound::Driver> SOUND;
