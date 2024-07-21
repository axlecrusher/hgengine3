#pragma once

#include <memory>

namespace HgSound {

	struct SamplePacket
	{
		SamplePacket()
			:sampleCount(0), sampleBytes(0), hasMorePackets(false), audioSamples(nullptr)
		{}

		size_t sampleCount; //number of consecutive samples
		size_t sampleBytes; //size of a single sample
		bool hasMorePackets;
		char* audioSamples;

		inline size_t ByteCount() const { return sampleCount * sampleBytes; }
	};

	struct SourceInfo
	{
		SourceInfo()
			:channels(0), sampleRate(0)
		{}

		uint8_t channels;
		uint32_t sampleRate;
	};

	class IAudioSourceState
	{
	public:
		virtual ~IAudioSourceState() {}
		virtual SourceInfo getSourceInfo() const = 0;
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
		inline SourceType getSourceType() const { return m_type; }
	protected:
		SourceType m_type;
	};

}