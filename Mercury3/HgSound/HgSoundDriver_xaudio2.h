#pragma once

#include <HgSound\HgSoundDriver.h>
#include <xaudio2.h>
#include <x3daudio.h>

#include <ProtectedVector.h>

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

struct Voice3D
{
	Voice3D()
		:pMatrixCoefficients(nullptr)
	{
	}

	~Voice3D()
	{
		if (pMatrixCoefficients) delete[] pMatrixCoefficients;
		pMatrixCoefficients = nullptr;
	}
	X3DAUDIO_EMITTER emitter;
	X3DAUDIO_DSP_SETTINGS dsp_settings;
	FLOAT32* pMatrixCoefficients;
};

struct Voice
{
	Voice()
		:xaudioVoice(nullptr)
	{}

	inline bool operator==(const VoiceCallback* rhs) const { return callback.get() == rhs; }
	inline bool operator<(const VoiceCallback* rhs) const { return callback.get() < rhs; }

	inline bool operator==(const PlayingSound* rhs) const { return sound.get() == rhs; }
	inline bool operator<(const PlayingSound* rhs) const { return sound.get() < rhs; }

	IXAudio2SourceVoice* xaudioVoice;
//	std::shared_ptr<IXAudio2SourceVoice> xaudioVoice;
	std::shared_ptr<VoiceCallback> callback;
	PlayingSound::ptr sound;
	std::shared_ptr<Voice3D> voice3d;
};

class XAudio2Driver : public HgSound::IDriver {
public:
	XAudio2Driver();
	~XAudio2Driver();
	virtual bool init();
	virtual bool start();
	virtual void shutdown();

	virtual void play(PlayingSound::ptr& sound, HgTime startOffset);
	virtual void play3d(PlayingSound::ptr& sound, const Emitter& emitter);
	virtual void stopPlayback(const PlayingSound* sound);

	virtual void update();

	void queueDestroy(VoiceCallback* x) { m_toDestroy.push_back(x); }

private:
	static void TimerClbk(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

	//void threadLoop();
	Voice initVoice(PlayingSound::ptr& sound);

	void InsertVoice(Voice& v);
	Voice RemoveVoice(VoiceCallback* x);

	void processDestroyQueue();
	void updateVoices();
	void update3DAudio();
	void startVoicePlaying(Voice& v);

	void processPlayEvents();


	bool m_initialized;
	IXAudio2* m_xaudioEngine;
	IXAudio2MasteringVoice* m_masteringVoice;
	XAUDIO2_VOICE_DETAILS m_masteringVoiceDetails;

	X3DAUDIO_HANDLE m_xaudio3d;
	//X3DAUDIO_LISTENER m_listener;

	std::mutex m_callbackMtx;
	std::vector<Voice> m_voices; //mutex protect

	ProtectedVector<VoiceCallback*> m_toDestroy;
	ProtectedVector<Events::PlaySound3D> m_playSoundEvents;

	std::atomic<bool> m_stop;
	//bool m_stop;
	//std::thread m_thread;

	//UINT m_timerId;

};
}
}