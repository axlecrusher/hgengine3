#include <HgSound\HgSoundDriver_xaudio2.h>
#include <Windows.h>
#include <iostream>
#include <algorithm>

namespace HgSound {
namespace XAudio2 {

XAudio2Driver::XAudio2Driver() : m_initialized(false), Driver() {
	m_xaudioEngine = nullptr;
	m_masteringVoice = nullptr;
	m_voices.reserve(32);
}

XAudio2Driver::~XAudio2Driver()
{
	stop(); //stop audio thread

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

	m_masteringVoice->GetVoiceDetails(&m_masteringVoiceDetails);

	//create 3D sound engine
	DWORD dwChannelMask;
	m_masteringVoice->GetChannelMask(&dwChannelMask);
	hr = X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, m_xaudio3d);
	if (hr != S_OK)
	{
		std::cerr << "Failed creating 3DAUDIO: " << hr << std::endl;
		return hr;
	}
//	memset(&m_listener, 0, sizeof(m_listener));

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
		update3DAudio();
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
		voice.sound->eventPlaybackEnded();
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

static void updateListener(const HgSound::Listener& listener, X3DAUDIO_LISTENER& x_listener)
{
	//negate Z component to transform to left hand system
	x_listener.Position.x = listener.getPosition().x();
	x_listener.Position.y = listener.getPosition().y();
	x_listener.Position.z = -listener.getPosition().z();

	const auto forward = listener.getForward();
	x_listener.OrientFront.x = forward.x();
	x_listener.OrientFront.y = forward.y();
	x_listener.OrientFront.z = -forward.z();

	const auto up = listener.getUp();
	x_listener.OrientTop.x = up.x();
	x_listener.OrientTop.y = up.y();
	x_listener.OrientTop.z = -up.z();
}

static void updateEmitter(const HgSound::Emitter& emitter, X3DAUDIO_EMITTER& x_emitter)
{
	//negate Z component to transform to left hand system
	x_emitter.Position.x = emitter.getPosition().x();
	x_emitter.Position.y = emitter.getPosition().y();
	x_emitter.Position.z = -emitter.getPosition().z();

	const auto forward = emitter.getForward();
	x_emitter.OrientFront.x = 0;
	x_emitter.OrientFront.y = 0;
	x_emitter.OrientFront.z = 1;

	const auto up = emitter.getUp();
	x_emitter.OrientTop.x = 0;
	x_emitter.OrientTop.y = 1;
	x_emitter.OrientTop.z = -0;
}

void XAudio2Driver::update3DAudio()
{
	X3DAUDIO_LISTENER x_listener;
	memset(&x_listener, 0, sizeof(X3DAUDIO_LISTENER));
	updateListener(getListener(), x_listener);

	//X3DAUDIO_EMITTER x_emitter;
	//memset(&x_emitter, 0, sizeof(X3DAUDIO_EMITTER));



	for (auto& voice : m_voices)
	{
		if (voice.voice3d)
		{
			auto& voice3d = voice.voice3d;
			auto& DSPSettings = voice3d->dsp_settings;

			updateEmitter(voice.sound->getEmitter(), voice3d->emitter);

			X3DAudioCalculate(m_xaudio3d, &x_listener, &voice3d->emitter,
				X3DAUDIO_CALCULATE_MATRIX |
				X3DAUDIO_CALCULATE_DOPPLER |
				X3DAUDIO_CALCULATE_LPF_DIRECT,
				&DSPSettings);

			voice.voice->SetOutputMatrix(m_masteringVoice, DSPSettings.SrcChannelCount, DSPSettings.DstChannelCount, DSPSettings.pMatrixCoefficients);
			voice.voice->SetFrequencyRatio(DSPSettings.DopplerFactor);
		}
	}
}

void XAudio2Driver::play(PlayingSound::ptr& sound, HgTime startOffset)
{
	Voice v = xplay(sound);
	InsertVoice(v);
	startVoicePlaying(v);
}

void XAudio2Driver::startVoicePlaying(Voice& v)
{
	auto hr = v.voice->Start(0);
	if (hr != S_OK)
	{
		std::cerr << "Failed to start playing sound: " << hr << std::endl;
	}
}

Voice XAudio2Driver::xplay(PlayingSound::ptr& sound)
{
	Voice v;

	auto asset = sound->getSoundAsset();

	WAVEFORMATEX format;
	format.cbSize = 0;
	format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	format.nChannels = asset->getNumChannels();
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
		return v;
	}

	hr = pSourceVoice->SubmitSourceBuffer(&buffer);
	if (hr != S_OK)
	{
		std::cerr << "Failed to play sound: " << hr << std::endl;
		return v;
	}

	pSourceVoice->SetVolume(sound->volume() * 0.1);

	v.voice = pSourceVoice;
	v.callback = voiceCallback;
	v.sound = sound;

//	InsertVoice(v);
//	InsertPlayingSound(sound);

	return v;
}

void XAudio2Driver::play3d(PlayingSound::ptr& sound, const Emitter& emitter)
{
	auto asset = sound->getSoundAsset();
	if (asset->getNumChannels() > 1)
	{
		std::cerr << "Can not play multichannel sound in 3D. Mono Only. " << std::endl;
		return;
	}

	Voice v = xplay(sound);

	X3DAUDIO_EMITTER x_emitter = { 0 };
	x_emitter.ChannelCount = asset->getNumChannels();
	x_emitter.CurveDistanceScaler = 100000.0;

	X3DAUDIO_DSP_SETTINGS DSPSettings = { 0 };
	DSPSettings.SrcChannelCount = asset->getNumChannels(); //number emitter channels
	DSPSettings.DstChannelCount = m_masteringVoiceDetails.InputChannels; //destination voice channels
	FLOAT32 * matrix = new FLOAT32[DSPSettings.SrcChannelCount*DSPSettings.DstChannelCount]; //SrcChannelCount * DstChannelCount
	DSPSettings.pMatrixCoefficients = matrix;

	updateEmitter(emitter, x_emitter);

	X3DAUDIO_LISTENER x_listener;
	memset(&x_listener, 0, sizeof(X3DAUDIO_LISTENER));
	updateListener(getListener(), x_listener);

	X3DAudioCalculate(m_xaudio3d, &x_listener, &x_emitter,
		X3DAUDIO_CALCULATE_MATRIX |
		X3DAUDIO_CALCULATE_DOPPLER |
		X3DAUDIO_CALCULATE_LPF_DIRECT,
		&DSPSettings);

	v.voice->SetOutputMatrix(m_masteringVoice, DSPSettings.SrcChannelCount, DSPSettings.DstChannelCount, DSPSettings.pMatrixCoefficients);
	v.voice->SetFrequencyRatio(DSPSettings.DopplerFactor);
	//pSFXSourceVoice->SetOutputMatrix(pSubmixVoice, 1, 1, &DSPSettings.ReverbLevel);

	startVoicePlaying(v);

	auto v3d = std::make_shared<Voice3D>();
	v3d->dsp_settings = DSPSettings;
	v3d->emitter = x_emitter;
	v.voice3d = v3d;

	sound->setEmitter(emitter);

	InsertVoice(v);
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