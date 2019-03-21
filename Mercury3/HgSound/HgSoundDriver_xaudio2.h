#pragma once

#include <HgSound\HgSoundDriver.h>
#include <xaudio2.h>
#include <x3daudio.h>

namespace HgSound {
namespace XAudio2 {

class XAudio2Driver;

class VoiceCallback : public IXAudio2VoiceCallback
{
public:
	VoiceCallback(XAudio2Driver* sd)
		: soundDriver(sd)
	{}

	virtual ~VoiceCallback() {  }

	//Called when the voice has just finished playing a contiguous audio stream.
	virtual void OnStreamEnd();

	//Unused methods are stubs
	virtual void OnBufferEnd(void * pBufferContext) {};
	virtual void OnBufferStart(void * pBufferContext) {}
	virtual void OnLoopEnd(void * pBufferContext) {}
	virtual void OnVoiceError(void * pBufferContext, HRESULT Error) {}
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) {    }
	void OnVoiceProcessingPassEnd() {}
private:
	XAudio2Driver* soundDriver;
};

struct Voice
{
	IXAudio2SourceVoice* voice;
	std::shared_ptr<VoiceCallback> callback;
	PlayingSound::ptr sound;
};

template<typename T>
class ProtectedVector
{
public:
	typedef  std::vector<T> vectorType;
	void push_back(T& x)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_values.push_back(x);
	}

	void swap(std::vector<T>& v)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_values.swap(v);
	}

private:
	std::mutex m_mutex;
	std::vector<T> m_values;
};

class XAudio2Driver : public Driver {
public:
	XAudio2Driver();
	~XAudio2Driver();
	virtual bool Init();
	virtual bool start();

	virtual void threadLoop();

	virtual void play(PlayingSound::ptr& asset, HgTime startOffset);

	void InsertVoice(Voice& v);

	void queueDestroy(VoiceCallback* x) { m_toDestroy.push_back(x); }

private:
	Voice RemoveVoice(VoiceCallback* x);
	void processDestroyQueue();
	void updateVoices();

	bool m_initialized;
	IXAudio2* m_xaudioEngine;
	IXAudio2MasteringVoice* m_masteringVoice;

	X3DAUDIO_HANDLE m_xaudio3d;
	X3DAUDIO_LISTENER m_listener;

	std::mutex m_callbackMtx;
	std::vector<Voice> m_voices; //mutex protect

	ProtectedVector<VoiceCallback*> m_toDestroy;
};
}
}