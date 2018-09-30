#include "HgSoundDriver.h"
#include <cstdio>
#include <math.h>

#include <HgMath.h>
#include <memory>

#include <HgSound\HgSoundDriver_libsoundio.h>

std::unique_ptr<HgSound::Driver> SOUND;

namespace HgSound {
	std::unique_ptr<HgSound::Driver> Driver::Create() { return std::make_unique<HgSound::LibSoundIoDriver>(); }

	Driver::Driver() : m_bufferSize(0), m_stop(false), m_continueMixing(false)
		//, m_initialized(false)
	{
	}

	Driver::~Driver()
	{
		stop();
	}

	static void startAudioLoop(Driver* driver) {
		Driver::audioLoop(driver);
	}

	void Driver::start() {
		m_thread = std::thread(startAudioLoop, this);
	}

	void Driver::stop() {
		//signal stop, wake thread from wait, wait for thread to join
		m_stop = true;
		continueExecution();
		if (m_thread.joinable())
			m_thread.join();
	}

	void Driver::audioLoop(Driver* driver) {
		while (!driver->m_stop) {
			driver->mixAudio();
			driver->wait();
		}
	}

	void Driver::wait() {
		m_continueMixing = false;
		std::unique_lock<std::mutex> lock(m_conditionMutex);
		while (!canContinue() && !m_stop) m_condition.wait(lock);
	}

	void Driver::continueExecution() {
		m_continueMixing = true;
		m_condition.notify_one();
	}

	PlayingSound::ptr Driver::play(SoundAsset::ptr& asset, HgTime startOffset) {
		if (asset == nullptr) return nullptr;

		auto sound = asset->play();
		if (sound == nullptr) return nullptr;
		const int32_t total_samples = samples * 2; //stereo


		sound->jumpToTime(startOffset);
		//sound->getSamples(total_samples, m_buffer);
		InsertPlayingSound(sound);
		return std::move(sound);
	}

	void Driver::InsertPlayingSound(PlayingSound::ptr& sound)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		m_playingSounds.insert(std::make_pair(sound.get(), sound));
	}

	PlayingSound::ptr Driver::RemovePlayingSound(PlayingSound::ptr& sound) {
		PlayingSound::ptr playingSound;
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		auto it = m_playingSounds.find(sound.get());
		if (it != m_playingSounds.end())
		{
			playingSound = it->second;
			m_playingSounds.erase(it);
			return playingSound;
		}
		return nullptr;
	}

	void Driver::stopPlayback(PlayingSound::ptr& playingAsset) {
		PlayingSound::ptr playingSound = RemovePlayingSound(playingAsset);
		if (playingSound != nullptr) {
			playingSound->eventPlaybackEnded();
		}
	}

	void Driver::mixAudio() {
		//if (!m_initialized) return;

		const int32_t total_samples = samples * 2; //stereo

		auto buffer = m_buffer.backBuffer();

		for (int32_t i = 0; i < total_samples; ++i) {
			buffer[i] = 0.0f;
		}

		{
			//figure out how to make this lock smaller...
			//the iterators make it hard to do
			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			for (auto it = m_playingSounds.begin(); it != m_playingSounds.end();) {
				auto this_iter = it++;
				auto playing = this_iter->second;
				playing->getSamples(total_samples, buffer);
				if (playing->isFinished()) {
					//can cause recursive mutex if receiver of event plays new audio.
					playing->eventPlaybackEnded();
					m_playingSounds.erase(this_iter);
				}
			}
		}

		for (int32_t i = 0; i < total_samples; ++i) {
			buffer[i] *= 0.1f;
		}

		m_buffer.swap();
	}
}