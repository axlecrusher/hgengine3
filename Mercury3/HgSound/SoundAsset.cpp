#include "SoundAsset.h"

#define DR_WAV_IMPLEMENTATION
#include <HgSound/dr_wav.h>
#include <HgSound/PlayingSound.h>

namespace HgSound {

	AssetManager<SoundAsset> SoundAsset::soundAssets;

	SoundAsset::SoundAsset() : m_data(nullptr), m_channels(0), m_sampleRate(0), m_totalSamples(0)
	{
	}


	SoundAsset::~SoundAsset()
	{
		if (m_data) delete[] m_data;
		m_data = nullptr;
	}

	SoundAsset::ptr SoundAsset::acquire(const std::string& path) {
		ptr tmp = soundAssets.get(path);
		tmp->selfPtr = tmp;
		return std::move(tmp);
	}

	bool SoundAsset::load(const std::string& path) {
		unsigned int channels, sampleRate;
		drwav_uint64 totalSampleCount;

		m_path = path;

		float* pSampleData = (float*)drwav_open_and_read_file_f32(path.c_str(), &channels, &sampleRate, &totalSampleCount);
		if (pSampleData == NULL) {
			return false;
		}

		m_sampleRate = sampleRate;
		m_channels = channels;
		m_totalSamples = totalSampleCount; //includes all channels
		m_sampleCount = totalSampleCount / channels;

		if (m_data) delete[] m_data;
		m_data = new float[totalSampleCount]; //new respects float alignment requirements
		memcpy(m_data, pSampleData, sizeof(float)*totalSampleCount);
		drwav_free(pSampleData);

		return true;
	}

	PlayingSound::ptr SoundAsset::play() {
		ptr tmp = selfPtr.lock();
		if (tmp) return std::move( std::make_shared<PlayingSound>(std::move(tmp)) );
		return nullptr;
	}
}