#pragma once

#include <HgSound\HgSoundDriver.h>

extern "C" {
#include <soundio\endian.h>
#include <soundio\soundio.h>
}
namespace HgSound {

	class LibSoundIoDriver : public Driver {
	public:
		LibSoundIoDriver();
		~LibSoundIoDriver();
		bool Init();
		bool start();
	private:
		typedef std::unique_ptr<SoundIo, decltype(&soundio_destroy)> soundio_ptr;
		typedef std::unique_ptr<SoundIoDevice, decltype(&soundio_device_unref)> device_ptr;
		typedef std::unique_ptr<SoundIoOutStream, decltype(&soundio_outstream_destroy)> outstream_ptr;

		soundio_ptr soundio{ nullptr,soundio_destroy };
		device_ptr device{ nullptr,soundio_device_unref };
		outstream_ptr outstream{ nullptr,soundio_outstream_destroy };

		static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max);

		bool m_initialized;
	};

}