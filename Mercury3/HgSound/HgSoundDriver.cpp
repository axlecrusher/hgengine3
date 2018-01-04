#include "HgSoundDriver.h"
#include <cstdio>
#include <math.h>

#include <HgMath.h>
#include <memory>

#include <HgSound\HgSoundDriver_libsoundio.h>

namespace HgSound {

	std::unique_ptr<HgSound::Driver> Driver::Create() { return std::make_unique<HgSound::LibSoundIoDriver>(); }
	
	PlayingSound::ptr Driver::play(SoundAsset::ptr asset) {
		PlayingSound::ptr tmp = asset->play();
		m_playingSounds.insert(std::make_pair(tmp.get(),tmp));
		return tmp;
	}

	void Driver::stop(PlayingSound::ptr playingAsset) {
		auto it = m_playingSounds.find(playingAsset.get());
		if (it != m_playingSounds.end()) m_playingSounds.erase(it);
	}

	Driver::Driver() : m_buffer(nullptr), m_bufferSize(0)
	{
	}

	Driver::~Driver() {
		if (m_buffer) delete m_buffer;
		m_buffer = nullptr;
	}

	void Driver::mixAudio() {
		uint32_t total_samples = samples * 2; //stereo

		for (uint32_t i = 0; i < total_samples; ++i) {
			m_buffer[i] = 0.0f;
		}

		for (auto it = m_playingSounds.begin(); it != m_playingSounds.end();) {
			auto this_iter = it++;
			auto playing = this_iter->second;
			playing->getSamples(total_samples, m_buffer);
			if (playing->isFinished()) m_playingSounds.erase(this_iter);
		}

		for (uint32_t i = 0; i < total_samples; ++i) {
			m_buffer[i] *= 0.1f;
		}
	}
}