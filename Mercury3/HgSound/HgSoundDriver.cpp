#include "HgSoundDriver.h"
#include <cstdio>
#include <math.h>

#include <HgMath.h>
#include <memory>

#include <HgSound\HgSoundDriver_libsoundio.h>

std::unique_ptr<HgSound::Driver> SOUND;

namespace HgSound {
	std::unique_ptr<HgSound::Driver> Driver::Create() { return std::make_unique<HgSound::LibSoundIoDriver>(); }

	Driver::Driver() : m_stop(false)
		//, m_initialized(false)
	{
		m_playingSounds.reserve(32);
		//m_tmpSounds.reserve(32);
	}

	Driver::~Driver()
	{
		stop();
	}

	void Driver::start() {
		m_thread = std::thread([](Driver* driver) { driver->mixingLoop(); }, this);
	}

	void Driver::stop() {
		//signal stop, wake thread from wait, wait for thread to join
		m_stop = true;
		continueExecution();
		if (m_thread.joinable())
			m_thread.join();
	}

	void Driver::mixingLoop() {
		while (!m_stop) {
			mixAudio();
			wait();
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
		PlayingSound::ptr ret;
		std::lock_guard<std::recursive_mutex> lock(m_mutex);

		auto it = std::find(m_playingSounds.begin(), m_playingSounds.end(), sound);
		if (it != m_playingSounds.end()) {
			ret = *it;
			m_playingSounds.erase(it);
		}
 		return ret;
	}

	void Driver::stopPlayback(PlayingSound::ptr& playingAsset) {
		PlayingSound::ptr playingSound = RemovePlayingSound(playingAsset);
		if (playingSound != nullptr) {
			playingSound->eventPlaybackEnded();
		}
	}

	auto Driver::PlayingSounds() {
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		return PlayingSoundList(m_playingSounds);
	}

	void Driver::mixAudio() {
		//if (!m_initialized) return;

		//copy m_playing sounds because other threads could be add and removing
		//while we are trying to mix sounds
		PlayingSoundList playingSounds = PlayingSounds();

		const int32_t total_samples = samples * 2; //stereo

		auto buffer = m_buffer.backBuffer();

		for (int32_t i = 0; i < total_samples; ++i) {
			buffer[i] = 0.0f;
		}

		for (auto& playing : playingSounds) {
			playing->getSamples(total_samples, buffer);
			if (playing->isFinished()) {
				stopPlayback(playing); //remove sound and call eventPlaybackEnded
			}
		}

		for (int32_t i = 0; i < total_samples; ++i) {
			buffer[i] *= 0.1f;
		}

		m_buffer.swap();
	}
}