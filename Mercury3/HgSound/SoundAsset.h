#pragma once

#include <AssetManager.h>
#include <HgSound/dr_wav.h>

namespace HgSound {
	class PlayingSound;
	class Driver;

	struct SamplePacket
	{
		uint32_t sampleCount; //number of consecutive samples
		uint32_t sampleBytes; //size of a single sample
		bool hasMorePackets;
		char* audioSamples;

		inline uint32_t ByteCount() const { return sampleCount * sampleBytes; }
	};

	class IAudioSourceState
	{
	public:
		virtual ~IAudioSourceState() {}
	};

	class IAudioSource
	{
	public:	
		
		enum SourceType
		{
			NONE,
			BUFFERED,
			STREAM
		};

		virtual ~IAudioSource() = default;
		virtual SamplePacket getBuffer(IAudioSourceState& state) const = 0;
		virtual std::unique_ptr<IAudioSourceState> newState() const = 0;

		inline SourceType sourceType() const { return m_type; }

	protected:
		SourceType m_type;
	};

	class BufferedWavSource : public IAudioSource
	{
	public:
		BufferedWavSource(float* samples, uint32_t sampleCount)
			: m_samples(samples), m_count(sampleCount)
		{
			m_type = SourceType::BUFFERED;
		}

		virtual ~BufferedWavSource();

		virtual SamplePacket getBuffer(IAudioSourceState& state) const;
		virtual std::unique_ptr<IAudioSourceState> newState() const;
	private:
		uint32_t m_count;
		float* m_samples;
	};

	class StreamingWavSource : public IAudioSource
	{
	public:
		class State : public IAudioSourceState
		{
		public:
			State()
				:m_buffer(nullptr)
			{}
			virtual ~State();

			void open(const char* path);
			inline drwav* get_drwav() { return &m_wav; }
			inline float* get_sampleBuffer() { return m_buffer; }

		private:
			drwav m_wav;
			//SamplePacket m_samples;

			float* m_buffer;
		};

		StreamingWavSource(std::string path)
			:m_path(path)
		{
			m_type = SourceType::STREAM;
		}

		virtual ~StreamingWavSource();

		virtual SamplePacket getBuffer(IAudioSourceState& state) const;
		virtual std::unique_ptr<IAudioSourceState> newState() const;
	private:
		std::string m_path;
	};

	class SoundAsset
	{
	public:


		typedef AssetManager<SoundAsset>::AssetPtr ptr;

		SoundAsset();

		static SoundAsset::ptr acquire(const std::string& path);

		//inline float* data() const { return m_audioSource->getBuffer(); }
		//inline SamplePacket getAudioData() const { return m_audioSource->getBuffer(); }
		inline IAudioSource& getAudioSource() const { return *m_audioSource; }

		uint8_t getNumChannels() const { return m_channels; }

		//total number of samples across all channels
		uint64_t totalSamples() const { return m_totalSamples; }

		//samples per second
		auto sampleRate() const { return m_sampleRate; }

		std::shared_ptr<PlayingSound> newPlayingInstance() { return play(); }

		inline auto getSourceType() const { return m_audioSource->sourceType(); }

		//returns true if an audio source is present
		inline bool isValid() const { return m_audioSource != nullptr; }

	private:
		std::string m_path;
		//std::unique_ptr<float[]> m_data;

		uint8_t m_channels;
		uint16_t m_sampleRate;
		uint64_t m_totalSamples; //all channels
		uint64_t m_frameCount;

		std::unique_ptr<IAudioSource> m_audioSource;

		std::weak_ptr<SoundAsset> selfPtr; //for creating shared pointers in play()

		bool load(const std::string& path);
		std::shared_ptr<PlayingSound> play();

		static AssetManager<SoundAsset> soundAssets;
		friend AssetManager<SoundAsset>;

		friend HgSound::Driver;
	};
}