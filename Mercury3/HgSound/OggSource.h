#pragma once

#include <string>
#include <HgSound/IAudioSource.h>
#include <vorbis/vorbisfile.h>

#include <future>

namespace HgSound {

	class StreamingOggSource : public IAudioSource
	{
	public:
		class State : public IAudioSourceState
		{
		public:
			State()
				:m_frontBuffer(nullptr), m_backBuffer(nullptr), m_samplesToRead(0), m_vorbisPtr(nullptr)
			{}
			virtual ~State();

			void open(const char* path);
			
			//Return C pointer to vorbis file. Can be nullptr.
			inline OggVorbis_File* get_vorbis() {
				if (m_vorbisPtr) { return m_vorbisPtr.get(); } return nullptr;
			}

			inline float* get_sampleBuffer() { return m_frontBuffer; }
			inline float* getBackBuffer() { return m_backBuffer; }

			void swapBuffers() { std::swap(m_frontBuffer, m_backBuffer); }

			static SamplePacket Decode(StreamingOggSource::State* state);
			//static bool isOgg(char* path);

			std::future<SamplePacket> nextToPlay;

			virtual SourceInfo getSourceInfo() const { return m_info; }

			SamplePacket currentPlaying;
		private:
			SourceInfo m_info;
			std::unique_ptr<OggVorbis_File> m_vorbisPtr;
			long m_samplesToRead;
			float *m_frontBuffer, *m_backBuffer;
		};

		StreamingOggSource(std::string path)
			:m_path(path)
		{
			m_type = SourceType::STREAM;
		}

		virtual ~StreamingOggSource();

		virtual SamplePacket getNextSamples(IAudioSourceState& state) const;
		virtual void initializeState(std::unique_ptr<IAudioSourceState>& state) const;

	private:
		std::string m_path;
	};

}