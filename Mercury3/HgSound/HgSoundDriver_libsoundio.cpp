#include <HgSound\HgSoundDriver_libsoundio.h>
#include <Windows.h>

namespace HgSound {

	enum errorEnum {
		ERR_UNDERFLOW,
		ERR_UNRECOVERABLE_STREAM,
		ERR_SMALL_WRITE
	};

	void handleError(errorEnum x, int err) {
		switch (x) {
		case ERR_UNRECOVERABLE_STREAM:
			fprintf(stderr, "unrecoverable stream error: %s\n", soundio_strerror(err));
			break;
		case ERR_SMALL_WRITE:
			fprintf(stderr, "small write: %d\n", err);
			break;
		}
	}


	static void underflow_callback(struct SoundIoOutStream *outstream) {
		static int count = 0;
		fprintf(stderr, "underflow %d\n", count++);
	}

	inline void write_sample_float32le(char *ptr, float sample) {
		float *buf = (float *)ptr;
		*buf = sample;
	}

	void LibSoundIoDriver::write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) {
		//this function must be realtime. Don't call blocking (I/O) things here.
		//	double float_sample_rate = outstream->sample_rate;
		//	printf("counts: %d %d\n", frame_count_min, frame_count_max);

		//must fill at least frame_count_min, unless frame_count_min=0. Then fill any amount up to frame_count_max
		//	double seconds_per_frame = 1.0 / float_sample_rate;
		struct SoundIoChannelArea *areas;
		int err;

		LibSoundIoDriver* driver = (LibSoundIoDriver*)outstream->userdata;
		//if (!driver->m_initialized) return;

		int frame_count = LibSoundIoDriver::samples;

		const struct SoundIoChannelLayout *layout = &outstream->layout;
		if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
			handleError(ERR_UNRECOVERABLE_STREAM, err);
			return;
		}

		if (frame_count < LibSoundIoDriver::samples) {
			handleError(ERR_SMALL_WRITE, frame_count);
		}


		//const auto buffer = driver->m_buffer.frontBuffer();
		driver->m_buffer.swapFrontBuffers();
		const auto buffer = driver->m_buffer.isolatedFrontBuffer();
		driver->continueExecution();
		
		const int channel_count = layout->channel_count;
		const auto stepSize = areas[0].step;

		int counterx = 0;
		for (int frame = 0; frame < frame_count; frame++) {
			for (int channel = 0; channel < channel_count; channel++) {
				write_sample_float32le(areas[channel].ptr, buffer[counterx++]);
				areas[channel].ptr += stepSize;
			}
		}

		if ((err = soundio_outstream_end_write(outstream))) {
			if (err == SoundIoErrorUnderflow)
			{
			}
			else {
				handleError(ERR_UNRECOVERABLE_STREAM, err);
				return;
			}
		}

		double latency;
		soundio_outstream_get_latency(outstream, &latency);
		//driver->continueExecution();
	}


	LibSoundIoDriver::LibSoundIoDriver() : m_initialized(false), Driver() {
	}

	LibSoundIoDriver::~LibSoundIoDriver() {
		//destroy in order
		outstream = nullptr;
		device = nullptr;
		soundio = nullptr;
	}

	bool LibSoundIoDriver::Init() {
		auto soundio = soundio_ptr(soundio_create(), soundio_destroy);

		if (!soundio) {
			fprintf(stderr, "out of memory\n");
			return false;
		}

		auto sound_ptr = soundio.get();
		int err = soundio_connect_backend(sound_ptr, SoundIoBackendWasapi); //windows

		if (err) {
			fprintf(stderr, "Unable to connect to sound backend: %s\n", soundio_strerror(err));
			return false;
		}

		fprintf(stderr, "Sound backend: %s\n", soundio_backend_name(soundio->current_backend));
		soundio_flush_events(sound_ptr);

		int selected_device_index = soundio_default_output_device_index(sound_ptr);
		if (selected_device_index < 0) {
			fprintf(stderr, "Output device not found\n");
			return false;
		}

		auto device = device_ptr(soundio_get_output_device(sound_ptr, selected_device_index), soundio_device_unref);
		if (!device) {
			fprintf(stderr, "out of memory\n");
			return false;
		}
		fprintf(stderr, "Sound output device: %s\n", device->name);
		if (device->probe_error) {
			fprintf(stderr, "Cannot probe device: %s\n", soundio_strerror(device->probe_error));
			return false;
		}

		auto outstream = outstream_ptr(soundio_outstream_create(device.get()), soundio_outstream_destroy);
		outstream->write_callback = LibSoundIoDriver::write_callback;
		outstream->underflow_callback = underflow_callback;
		outstream->name = NULL;
		double latency = 1.0 / 100.0;
		int sample_rate = 44100;
		outstream->software_latency = latency;
		outstream->sample_rate = sample_rate;
		outstream->userdata = this;
		if (soundio_device_supports_format(device.get(), SoundIoFormatFloat32LE)) {
			outstream->format = SoundIoFormatFloat32LE;
			fprintf(stderr, "SoundIoFormatFloat32LE\n");
			//			write_sample = write_sample_float32le;
		}
		//else if (soundio_device_supports_format(device.get(), SoundIoFormatS16LE)) {
		//	outstream->format = SoundIoFormatS16LE;
		//	//		write_sample = write_sample_s16ne;
		//}
		else {
			fprintf(stderr, "No suitable device format available.\n");
			return false;
		}

		uint8_t channels = 2; //stereo
		const uint32_t bufferSize = channels * samples;
		m_buffer.allocate(bufferSize); //allocate and init to 0

		//Move to after buffer init?
		if ((err = soundio_outstream_open(outstream.get()))) {
			fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
			return false;
		}
		fprintf(stderr, "Software latency: %f\n", outstream->software_latency);

		if (outstream->layout_error)
			fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));

		this->soundio = std::move(soundio);
		this->device = std::move(device);
		this->outstream = std::move(outstream);

		m_initialized = true;

		return true;
	}

	bool LibSoundIoDriver::start() {
		bool success = Driver::start();

		if (!m_initialized)
		{
			fprintf(stderr, "LibSoundIoDriver uninitialized\n");
			return false;
		}

		int err;

		if ((err = soundio_outstream_start(outstream.get()))) {
			fprintf(stderr, "unable to start device: %s\n", soundio_strerror(err));
			return false;
		}

		return success;
	}
}