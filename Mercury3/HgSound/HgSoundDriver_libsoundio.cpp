#include <HgSound\HgSoundDriver_libsoundio.h>
#include <Windows.h>

namespace HgSound {


	static void underflow_callback(struct SoundIoOutStream *outstream) {
		static int count = 0;
		fprintf(stderr, "underflow %d\n", count++);
	}

//	static volatile bool want_pause = false;

	static void(*write_sample)(char *ptr, double sample);

	uint32_t Driver::samples = 441; //for each channel

	static void write_sample_float32le(char *ptr, double sample) {
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
		int frames_left = LibSoundIoDriver::samples;
		uint32_t counterx = 0;

		LibSoundIoDriver* driver = (LibSoundIoDriver*)outstream->userdata;
		if (!driver->m_initialized) return;


		int frame_count = frames_left;

		const struct SoundIoChannelLayout *layout = &outstream->layout;

		if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
			fprintf(stderr, "unrecoverable stream error: %s\n", soundio_strerror(err));
			return;
		}

		//mixAudio probably should be moved out of write_callback into its own thread
		//as write_callback needs to be fast. Will need some kind of thread sync.
		//should probably also double buffer mixed audio. Or have a larger circular buffer
		//with a moving play head. New playing audio could be mixed in at the play
		//head for low latency.
		driver->mixAudio();

		for (uint32_t frame = 0; frame < frame_count; frame++) {
			for (int channel = 0; channel < layout->channel_count; channel++) {
				write_sample(areas[channel].ptr, driver->m_buffer[counterx++]);
				areas[channel].ptr += areas[channel].step;
			}
		}

		if ((err = soundio_outstream_end_write(outstream))) {
			if (err == SoundIoErrorUnderflow)
				return;
			fprintf(stderr, "unrecoverable stream error: %s\n", soundio_strerror(err));
			return;
		}

		/*
		double out_latency;
		soundio_outstream_get_latency(outstream, &out_latency);
		printf("latency %f\n", out_latency);
		*/
		//	soundio_outstream_pause(outstreamw, want_pause); //what is this for?
	}


	LibSoundIoDriver::LibSoundIoDriver() : Driver() {
	}

	LibSoundIoDriver::~LibSoundIoDriver() {
		//destroy in order
		outstream = nullptr;
		device = nullptr;
		soundio = nullptr;

		if (m_buffer != nullptr) delete m_buffer;
		m_buffer = nullptr;
	}

	void LibSoundIoDriver::Init() {
		auto soundio = soundio_ptr(soundio_create(), soundio_destroy);

		if (!soundio) {
			fprintf(stderr, "out of memory\n");
			return;
		}

		auto sound_ptr = soundio.get();
		int err = soundio_connect_backend(sound_ptr, SoundIoBackendWasapi); //windows

		if (err) {
			fprintf(stderr, "Unable to connect to sound backend: %s\n", soundio_strerror(err));
			return;
		}

		fprintf(stderr, "Sound backend: %s\n", soundio_backend_name(soundio->current_backend));
		soundio_flush_events(sound_ptr);

		int selected_device_index = soundio_default_output_device_index(sound_ptr);
		if (selected_device_index < 0) {
			fprintf(stderr, "Output device not found\n");
			return;
		}

		auto device = device_ptr(soundio_get_output_device(sound_ptr, selected_device_index), soundio_device_unref);
		if (!device) {
			fprintf(stderr, "out of memory\n");
			return;
		}
		fprintf(stderr, "Sound output device: %s\n", device->name);
		if (device->probe_error) {
			fprintf(stderr, "Cannot probe device: %s\n", soundio_strerror(device->probe_error));
			return;
		}

		auto outstream = outstream_ptr(soundio_outstream_create(device.get()), soundio_outstream_destroy);
		outstream->write_callback = LibSoundIoDriver::write_callback;
		outstream->underflow_callback = underflow_callback;
		outstream->name = NULL;
		double latency = 100;
		int sample_rate = 44100;
		outstream->software_latency = latency;
		outstream->sample_rate = sample_rate;
		outstream->userdata = this;
		if (soundio_device_supports_format(device.get(), SoundIoFormatFloat32LE)) {
			outstream->format = SoundIoFormatFloat32LE;
			fprintf(stderr, "SoundIoFormatFloat32LE\n");
			write_sample = write_sample_float32le;
		}
		else if (soundio_device_supports_format(device.get(), SoundIoFormatS16LE)) {
			outstream->format = SoundIoFormatS16LE;
			//		write_sample = write_sample_s16ne;
		}
		else {
			fprintf(stderr, "No suitable device format available.\n");
			return;
		}


		//Move to after buffer init?
		if ((err = soundio_outstream_open(outstream.get()))) {
			fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
			return;
		}
		fprintf(stderr, "Software latency: %f\n", outstream->software_latency);

		if (outstream->layout_error)
			fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));
		if ((err = soundio_outstream_start(outstream.get()))) {
			fprintf(stderr, "unable to start device: %s\n", soundio_strerror(err));
			return;
		}

		uint8_t channels = 2; //stereo
		m_bufferSize = channels*samples;
		m_buffer = new float[m_bufferSize];

		this->soundio = std::move(soundio);
		this->device = std::move(device);
		this->outstream = std::move(outstream);

		m_initialized = true;
	}

}