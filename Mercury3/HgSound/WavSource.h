#pragma once

#include <string>
#include <HgSound/dr_wav.h>
#include <HgSound/IAudioSource.h>

#include <future>

namespace HgSound {

	class BufferedWavSource : public IAudioSource
	{
	public:
		BufferedWavSource(const SourceInfo& si, float* samples, uint32_t sampleCount)
			: m_sourceInfo(si), m_samples(samples), m_count(sampleCount)
		{
			m_type = SourceType::BUFFERED;
		}

		class State : public IAudioSourceState
		{
		public:
			State(const SourceInfo& si)
				:m_sourceInfo(si)
			{}
			SourceInfo getSourceInfo() const { return m_sourceInfo; }
		private:
			const SourceInfo& m_sourceInfo;
		};

		virtual ~BufferedWavSource();

		virtual SamplePacket getNextSamples(IAudioSourceState& state) const;
		virtual void initializeState(std::unique_ptr<IAudioSourceState>& state) const;
	private:
		SourceInfo m_sourceInfo;

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
				:m_frontBuffer(nullptr), m_backBuffer(nullptr)
			{}
			virtual ~State();

			void open(const char* path);
			inline drwav* get_drwav() { return &m_wav; }
			inline float* get_sampleBuffer() { return m_frontBuffer; }
			inline float* getBackBuffer() { return m_backBuffer; }
			virtual SourceInfo getSourceInfo() const { return m_info; }

			void swapBuffers() { std::swap(m_frontBuffer, m_backBuffer); }

			std::future<SamplePacket> nextToPlay;

			SamplePacket currentPlaying;
		private:
			SourceInfo m_info;
			drwav m_wav;
			float *m_frontBuffer, *m_backBuffer;
		};

		StreamingWavSource(std::string path)
			:m_path(path)
		{
			m_type = SourceType::STREAM;
		}

		virtual ~StreamingWavSource();

		virtual SamplePacket getNextSamples(IAudioSourceState& state) const;
		virtual void initializeState(std::unique_ptr<IAudioSourceState>& state) const;
	private:
		std::string m_path;
	};

}