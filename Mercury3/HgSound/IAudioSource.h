#pragma once

namespace HgSound {

	struct SamplePacket
	{
		size_t sampleCount; //number of consecutive samples
		size_t sampleBytes; //size of a single sample
		bool hasMorePackets;
		char* audioSamples;

		inline size_t ByteCount() const { return sampleCount * sampleBytes; }
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
		virtual SamplePacket getNextSamples(IAudioSourceState& state) const = 0;
		virtual void initializeState(std::unique_ptr<IAudioSourceState>& state) const = 0;

		inline SourceType sourceType() const { return m_type; }

	protected:
		SourceType m_type;
	};

}