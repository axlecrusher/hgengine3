#include <HgSound\HgSoundDriver_xaudio2.h>
#include <Windows.h>
#include <iostream>

namespace HgSound {
namespace XAudio2 {

XAudio2Driver::XAudio2Driver() : m_initialized(false), Driver() {
	m_xaudioEngine = nullptr;
	m_masteringVoice = nullptr;
	m_voices.reserve(32);
}

XAudio2Driver::~XAudio2Driver()
{
	if (m_xaudioEngine)
	{
		m_xaudioEngine->Release();
		m_xaudioEngine = nullptr;
	}
}

bool XAudio2Driver::Init() {
	HRESULT hr;
	hr = XAudio2Create(&m_xaudioEngine, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (hr != S_OK)
	{
		std::cerr << "Failed creating xaudioEngine: " << hr << std::endl;
		return false;
	}

	hr = m_xaudioEngine->CreateMasteringVoice(&m_masteringVoice);
	if (hr != S_OK)
	{
		std::cerr << "Failed creating mastering voice: " << hr << std::endl;
		return hr;
	}

	//create 3D sound engine
	DWORD dwChannelMask;
	m_masteringVoice->GetChannelMask(&dwChannelMask);
	X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, m_xaudio3d);
	memset(&m_listener, 0, sizeof(m_listener));

	return true;
}

bool XAudio2Driver::start() {
	bool success = Driver::start();

	return success;
	return false;
}

void XAudio2Driver::threadLoop()
{
	while (!stopping())
	{
		Sleep(10);

		processDestroyQueue();
//		updateVoices();
	}
}

void XAudio2Driver::processDestroyQueue()
{
	decltype(m_toDestroy)::vectorType tmp;
	m_toDestroy.swap(tmp);

	for (auto clbk : tmp)
	{
		auto voice = RemoveVoice(clbk);
		voice.voice->DestroyVoice();
	}
}

void XAudio2Driver::updateVoices()
{
	std::lock_guard<std::mutex> lock(m_callbackMtx);
	for (auto& voice : m_voices)
	{
		auto sound = voice.sound;
		voice.voice->SetVolume(sound->volume() * 0.1);
	}

}

void XAudio2Driver::play(PlayingSound::ptr& sound, HgTime startOffset)
{
	auto asset = sound->getSoundAsset();

	WAVEFORMATEX format;
	format.cbSize = 0;
	format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	format.nChannels = asset->channels();
	format.nSamplesPerSec = asset->sampleRate();
	format.nAvgBytesPerSec = 4 * format.nChannels * format.nSamplesPerSec;
	format.wBitsPerSample = 32;
	format.nBlockAlign = (format.nChannels*format.wBitsPerSample) / 8;

	XAUDIO2_BUFFER buffer;
	memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));

	buffer.AudioBytes = asset->totalSamples() * 4;
	buffer.pAudioData = reinterpret_cast<const BYTE*>(asset->data());
	buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	HRESULT hr;
	IXAudio2SourceVoice* pSourceVoice;
	auto voiceCallback = std::make_shared<VoiceCallback>(this);

	hr = m_xaudioEngine->CreateSourceVoice(&pSourceVoice, &format, 0,
		XAUDIO2_DEFAULT_FREQ_RATIO, voiceCallback.get(), NULL, NULL);
	if (hr != S_OK)
	{
		std::cerr << "Failed to create source voice: " << hr << std::endl;
		return;
	}

	hr = pSourceVoice->SubmitSourceBuffer(&buffer);
	if (hr != S_OK)
	{
		std::cerr << "Failed to play sound: " << hr << std::endl;
		return;
	}

	pSourceVoice->SetVolume(sound->volume() * 0.1);
	hr = pSourceVoice->Start(0);
	if (hr != S_OK)
	{
		std::cerr << "Failed to start playing sound: " << hr << std::endl;
		return;
	}

	Voice v;
	v.voice = pSourceVoice;
	v.callback = voiceCallback;
	v.sound = sound;

	InsertVoice(v);
//	InsertPlayingSound(sound);
}

void XAudio2Driver::InsertVoice(Voice& v)
{
	std::lock_guard<std::mutex> lock(m_callbackMtx);
	m_voices.push_back(v);
}


Voice XAudio2Driver::RemoveVoice(VoiceCallback* x)
{
	std::lock_guard<std::mutex> lock(m_callbackMtx);
	Voice ret;
	for (auto& voice : m_voices)
	{
		if (voice.callback.get() == x)
		{
			ret = voice;
			voice = m_voices.back();
			m_voices.pop_back();
			return ret;
		}
	}
	return ret;
}

void VoiceCallback::OnStreamEnd()
{
	//std::cout << "stream ended" << std::endl;
	soundDriver->queueDestroy(this);
}

}} //namespaces