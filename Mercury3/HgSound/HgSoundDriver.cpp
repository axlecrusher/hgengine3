#include "HgSoundDriver.h"
#include <cstdio>
#include <math.h>

#include <HgMath.h>
#include <memory>

#include <HgSound\HgSoundDriver_libsoundio.h>

std::unique_ptr<HgSound::Driver> SOUND;

namespace HgSound {

	std::unique_ptr<HgSound::Driver> Driver::Create() { return std::make_unique<HgSound::LibSoundIoDriver>(); }
	
	void Driver::threadLoop(Driver* driver) {
		while (!driver->m_stopThread) {
			{
				std::unique_lock<std::mutex> tmp(driver->m_mixMutex);
				driver->m_mixWait.wait(tmp);
			}
			if (!driver->m_stopThread) driver->mixAudio();
		}
	}

	void Driver::start() {
		m_stopThread = false;
		m_mixThread = std::thread(threadLoop, this);
	}

	PlayingSound::ptr Driver::play(SoundAsset::ptr asset) {
		PlayingSound::ptr tmp = asset->play();
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_playingSounds.insert(std::make_pair(tmp.get(), tmp));
		}
		return std::move(tmp);
	}

	void Driver::stop(PlayingSound::ptr playingAsset) {
		PlayingSound::ptr playingSound;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			auto it = m_playingSounds.find(playingAsset.get());
			if (it != m_playingSounds.end())
			{
				playingSound = it->second;
				m_playingSounds.erase(it);
			}
		}
		if (playingSound != nullptr) {
			playingSound->eventPlaybackEnded();
		}
	}

	Driver::Driver() : m_buffer(nullptr), m_bufferSize(0), m_initialized(false), m_stopThread(true)
	{
	}

	Driver::~Driver() {
		stopThread();

		if (m_buffer) delete m_buffer;
		m_buffer = nullptr;
	}

	void Driver::mixAudio() {
		if (!m_initialized) return;

		uint32_t total_samples = samples * 2; //stereo

		for (uint32_t i = 0; i < total_samples; ++i) {
			m_buffer[i] = 0.0f;
		}

		{
			//figure out how to make this lock smaller...
			//the iterators make it hard to do
			std::lock_guard<std::mutex> lock(m_mutex);
			for (auto it = m_playingSounds.begin(); it != m_playingSounds.end();) {
				auto this_iter = it++;
				auto playing = this_iter->second;
				playing->getSamples(total_samples, m_buffer);
				if (playing->isFinished()) {
					playing->eventPlaybackEnded();
					m_playingSounds.erase(this_iter);
				}
			}
		}

		for (uint32_t i = 0; i < total_samples; ++i) {
			m_buffer[i] *= 0.1f;
		}
	}
}