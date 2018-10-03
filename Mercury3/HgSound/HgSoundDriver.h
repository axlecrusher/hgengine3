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

		void continueExecution() { m_wait.resume(); }
	protected:
		const static int32_t samples;

		void mixAudio();

		DoubleBuffer<float> m_buffer;
		uint32_t m_bufferSize;

	private:
		void audioLoop();
		void wait() { m_wait.wait(); }
		void InsertPlayingSound(PlayingSound::ptr& sound);
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
