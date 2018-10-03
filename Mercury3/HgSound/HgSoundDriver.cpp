#include "HgSoundDriver.h"
#include <cstdio>
#include <math.h>

#include <HgMath.h>
#include <memory>

#include <HgSound\HgSoundDriver_libsoundio.h>

std::unique_ptr<HgSound::Driver> SOUND;

namespace HgSound {
	std::unique_ptr<HgSound::Driver> Driver::Create() { return std::make_unique<HgSound::LibSoundIoDriver>(); }

	Driver::Driver() : m_bufferSize(0), m_stop(false)
		//, m_initialized(false)
	{
		m_playingSounds.reserve(32);
		//m_tmpSounds.reserve(32);
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
		m_playingSounds.push_back(sound);
	}

	PlayingSound::ptr Driver::RemovePlayingSound(PlayingSound::ptr& sound) {
		PlayingSound::ptr playingSound;
		PlayingSoundList playingSounds, tmpList;

		atomic_swap(m_playingSounds, playingSounds, m_mutex);

		const auto count = playingSounds.size();
		tmpList.reserve(count);

		for (int i = 0; i < count; ++i) {
			auto& playing = playingSounds[i];
			if (playing.get() != sound.get()) {
				tmpList.push_back(std::move(playing));
			}
			else {
				playingSound = playing;
			}
		}

		//preserve any playing sounds that were added during this function call
		atomic_concat(tmpList, m_playingSounds, m_mutex);

		return playingSound;
	}

	void Driver::stopPlayback(PlayingSound::ptr& playingAsset) {
		PlayingSound::ptr playingSound = RemovePlayingSound(playingAsset);
		if (playingSound != nullptr) {
			playingSound->eventPlaybackEnded();
		}
	}

	void Driver::mixAudio() {
		//if (!m_initialized) return;
		PlayingSoundList playingSounds, tmpList;

		const int32_t total_samples = samples * 2; //stereo

		auto buffer = m_buffer.backBuffer();

		for (int32_t i = 0; i < total_samples; ++i) {
			buffer[i] = 0.0f;
		}

		//swap into local playingSounds so we don't have to worry about locking for a long time 
		atomic_swap(playingSounds, m_playingSounds, m_mutex);

		const auto count = playingSounds.size();
		tmpList.reserve(count);

		for (int i = 0; i < count; ++i) {
			auto& playing = playingSounds[i];
			playing->getSamples(total_samples, buffer);
			if (playing->isFinished()) {
				playing->eventPlaybackEnded();
			}
			else {
				tmpList.push_back(std::move(playing));
			}
		}

		//preserve any playing sounds that were added during this function call
		atomic_concat(tmpList, m_playingSounds, m_mutex);

		for (int32_t i = 0; i < total_samples; ++i) {
			buffer[i] *= 0.1f;
		}

		m_buffer.swap();
	}
}