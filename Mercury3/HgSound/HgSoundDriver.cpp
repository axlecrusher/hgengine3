#include "HgSoundDriver.h"
#include <cstdio>
#include <math.h>

#include <HgMath.h>
#include <memory>

#include <HgSound\HgSoundDriver_libsoundio.h>
#include <HgSound\HgSoundDriver_xaudio2.h>
#include <EventSystem.h>

std::unique_ptr<HgSound::IDriver> SOUND;

namespace HgSound {
	const int32_t Driver::samples = 441 * 2; //for each channel, 10ms

std::unique_ptr<HgSound::IDriver> Create()
//void Create()
{
	//std::unique_ptr<HgSound::IDriver> sound_service = std::make_unique<HgSound::XAudio2::XAudio2Driver>();
	//ServiceLocator<IDriver>::provide(sound_service);
	return std::make_unique<HgSound::XAudio2::XAudio2Driver>();
}

	Driver::Driver() : m_stop(false)
		//, m_initialized(false)
	{
		m_playingSounds.reserve(32);
		//m_tmpSounds.reserve(32);
	}

	Driver::~Driver()
	{
		shutdown();
	}

	bool Driver::start() {
		m_thread = std::thread([](Driver* driver) { driver->threadLoop(); }, this);
		return true;
	}

	void Driver::shutdown() {
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

	void Driver::play(PlayingSound::ptr& sound, HgTime startOffset) {
		if (sound == nullptr) return;

		auto asset = sound->getSoundAsset();
		const int32_t total_samples = samples * asset->getNumChannels(); //stereo

//		sound->jumpToTime(startOffset);
		//sound->getSamples(total_samples, m_buffer);
		InsertPlayingSound(sound);
	}

	void Driver::InsertPlayingSound(PlayingSound::ptr& sound)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		m_playingSounds.push_back(sound);
	}

	PlayingSound::ptr Driver::RemovePlayingSound(const PlayingSound* sound) {
		PlayingSound::ptr ret;
		std::lock_guard<std::recursive_mutex> lock(m_mutex);

//		auto it = std::find(m_playingSounds.begin(), m_playingSounds.end(), sound);
		auto it = std::find_if(m_playingSounds.begin(), m_playingSounds.end(),
			[sound](auto ptr) { return ptr.get() == sound;}
		);

		if (it != m_playingSounds.end()) {
			ret = *it;
			m_playingSounds.erase(it);
		}
 		return ret;
	}

	void Driver::stopPlayback(const PlayingSound* playingAsset) {
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
//FIXME			playing->getSamples(total_samples, buffer);
//FIXME			if (playing->isFinished()) {
				stopPlayback(playing.get()); //remove sound and call eventPlaybackEnded
//			}
		}

		for (int32_t i = 0; i < total_samples; ++i) {
			buffer[i] *= 0.1f;
		}

		m_buffer.swap();
	}
}

REGISTER_EVENT_TYPE(HgSound::Events::PlaySound3D);