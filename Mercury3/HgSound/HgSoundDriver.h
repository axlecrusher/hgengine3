#pragma once

#include "lib\libsoundio-1.1.0\soundio\soundio.h"
#include <memory>
#include <vector>
#include <HgSound/SoundAsset.h>
#include <HgSound/PlayingSound.h>

namespace HgSound {

	class Driver {
	public:
		Driver();
		virtual ~Driver();
		virtual void Init() = 0;

		virtual void start() = 0;

		PlayingSound::ptr play(SoundAsset::ptr asset);
		void stop(PlayingSound::ptr playingAsset);
	protected:
		std::map<const PlayingSound*, PlayingSound::ptr> m_playingSounds;
		static uint32_t samples;

		void mixAudio();

		float* m_buffer;
		uint32_t m_bufferSize;
	};

	class LibSoundIoDriver : public Driver {
	public:
		LibSoundIoDriver();
		~LibSoundIoDriver();
		void Init();

		void start() {}
	private:
		typedef std::unique_ptr<SoundIo, decltype(&soundio_destroy)> soundio_ptr;
		typedef std::unique_ptr<SoundIoDevice, decltype(&soundio_device_unref)> device_ptr;
		typedef std::unique_ptr<SoundIoOutStream, decltype(&soundio_outstream_destroy)> outstream_ptr;

		soundio_ptr soundio{ nullptr,soundio_destroy };
		device_ptr device{ nullptr,soundio_device_unref };
		outstream_ptr outstream{ nullptr,soundio_outstream_destroy };

		static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max);
	};
}