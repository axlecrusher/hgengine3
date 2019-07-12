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

#include <Emitter.h>

namespace HgSound {

class IDriver
{
public:
	IDriver()
	{}

	virtual ~IDriver()
	{}

	virtual bool init() = 0;

	virtual bool start() = 0;

	virtual void shutdown() = 0;

	virtual void play(PlayingSound::ptr& sound, HgTime startOffset) = 0;

	virtual void play3d(PlayingSound::ptr& sound, const Emitter& emitter) = 0;

	void play(PlayingSound::ptr& sound) {
		const auto zero = HgTime::msec(0);
		return play(sound, zero);
	}

	virtual void stopPlayback(const PlayingSound* sound) = 0;

	Listener getListener() const { return m_listener; }
	void setListener(const Listener& l) { m_listener = l; }

	//perform update of playing sounds. Intended to run from within game loop
	virtual void update() = 0;

private:

	Listener m_listener;
};

std::unique_ptr<HgSound::IDriver> Create();
//void Create();

//Driver is very closely tied to how libsoundio is expected to work. just rework
//into libsoundio driver
	class Driver {
	public:
		Driver();
		virtual ~Driver();
		virtual bool Init() = 0;

		//start audio decoding and mixing thread
		virtual bool start();

		virtual void threadLoop() = 0;

		virtual void play(PlayingSound::ptr& sound, HgTime startOffset);

		virtual void play3d(PlayingSound::ptr& sound, const Emitter& emitter) = 0;

		void play(PlayingSound::ptr& sound) {
			const auto zero = HgTime::msec(0);
			return play(sound, zero);
		}

		void shutdown();

		virtual void stopPlayback(const PlayingSound* sound);

		void mixingLoop();

		void continueExecution() { m_wait.resume(); }

		Listener getListener() const { return m_listener; }
		void setListener(const Listener& l) { m_listener = l; }

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
		PlayingSound::ptr RemovePlayingSound(const PlayingSound* sound);

		bool m_stop;
		std::thread m_thread;

		typedef std::vector<PlayingSound::ptr> PlayingSoundList;
		std::vector< PlayingSound::ptr> m_playingSounds;
		//std::vector< PlayingSound::ptr> m_tmpSounds; //accumulates still playing sounds
		std::recursive_mutex m_mutex;

		ConditionalWait m_wait;

		Listener m_listener;
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

	namespace Events
	{

	struct PlaySound3D
	{
		EntityIdType emittingEntityId;
		HgSound::Emitter emitter;
		std::string filename;
		float volume;
	};

	}
}

extern std::unique_ptr<HgSound::IDriver> SOUND;
