#include <HgSound\HgSoundDriver_xaudio2.h>
#include <Windows.h>
#include <iostream>
#include <algorithm>

#include <EventSystem.h>

namespace HgSound {
namespace XAudio2 {

//lower volume to allow for more voices
static const float VolumeMultiplier = 0.1;

XAudio2Driver::XAudio2Driver() : m_initialized(false), IDriver() {
	m_xaudioEngine = nullptr;
	m_masteringVoice = nullptr;
	m_voices.reserve(32);
	m_3dvoices.reserve(32);
	m_stop = false;
}

XAudio2Driver::~XAudio2Driver()
{
	shutdown(); //stop audio thread
}

bool XAudio2Driver::init() {

	EventSystem::Register<Events::PlaySound3D>(this, [this](const Events::PlaySound3D& evt) {
		m_playSoundEvents.push_back(Events::PlaySound3D(evt));
	});

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

XAudio2Driver::ClbkUserData::ClbkUserData()
{
	timerId = 0;
	driver = nullptr;
}

void XAudio2Driver::TimerClbk(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	if (dwUser == NULL) return;

	const auto data = (const ClbkUserData*)dwUser;
	if (data->magic == 0x58415532)
	{
		if (data->timerId == uTimerID)
		{
			XAudio2Driver* driver = data->driver;
			driver->processPlayEvents();
			driver->processDestroyQueue();
			driver->update3DAudio();
			//		updateVoices();
			driver->m_stop = false;
		}
	}
}

bool XAudio2Driver::start() {
	//bool success = Driver::start();
	//m_thread = std::thread([](XAudio2Driver* driver) { driver->threadLoop(); }, this);

	TIMECAPS tc;
	UINT     wTimerRes;

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
	{
		// Error; application can't continue.
	}

	wTimerRes = (std::min)((std::max)(tc.wPeriodMin, 2u), tc.wPeriodMax);
	timeBeginPeriod(wTimerRes);

	m_clbkData.driver = this;
	m_clbkData.timerId = timeSetEvent(10, 4, TimerClbk, (DWORD_PTR)&m_clbkData, TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);

	timeEndPeriod(wTimerRes);

	return true;
}

void XAudio2Driver::shutdown()
{
	timeKillEvent(m_clbkData.timerId);

	//make sure TimerClbk isn't running??
	while (m_stop == false)
	{
		m_stop = true;
		Sleep(100);
	}


	//signal stop, wake thread from wait, wait for thread to join
	m_stop = true;
	//if (m_thread.joinable())
	//	m_thread.join();

	if (m_xaudioEngine)
	{
		m_xaudioEngine->Release();
		m_xaudioEngine = nullptr;
	}
}

//void XAudio2Driver::threadLoop()
//{
//	while (!m_stop)
//	{
//		Sleep(10);
//
//		processPlayEvents();
//		processDestroyQueue();
//		update3DAudio();
////		updateVoices();
//	}
//}

void XAudio2Driver::processPlayEvents()
{
	decltype(m_playSoundEvents)::vectorType tmp;
	m_playSoundEvents.swap(tmp);

	for (auto& evt : tmp)
	{
		auto asset = HgSound::SoundAsset::acquire(evt.filename);
		if (asset->isValid())
		{
			auto snd = asset->newPlayingInstance();
			snd->setVolume(evt.volume);
			if ( EntityIdTable::Singleton().exists(evt.emittingEntityId) )
			{
				auto entity = HgEntity::Find(evt.emittingEntityId);
				evt.emitter = snd->EmitFromEntity(entity.entity);
			}
			SOUND->play3d(snd, evt.emitter);
		}
	}
}


void XAudio2Driver::processDestroyQueue()
{
	decltype(m_toDestroy)::vectorType tmp;
	m_toDestroy.swap(tmp);

	for (auto clbk : tmp)
	{
		auto voice = RemoveVoice(clbk);
		voice->xaudioVoice->DestroyVoice();
		voice->sound->eventPlaybackEnded();
	}
}

void XAudio2Driver::updateVoices()
{
	std::lock_guard<std::mutex> lock(m_callbackMtx);
	for (auto& voice : m_voices)
	{
		auto sound = voice->sound;
		voice->xaudioVoice->SetVolume(sound->getVolume() * VolumeMultiplier);
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



	for (auto& voice : m_3dvoices)
	{
		auto& sound = voice->sound;

		auto& voice3d = voice->voice3d;
		auto& DSPSettings = voice3d->dsp_settings;

		updateEmitter(sound->getEmitter(), voice3d->emitter);

		X3DAudioCalculate(m_xaudio3d, &x_listener, &voice3d->emitter,
			X3DAUDIO_CALCULATE_MATRIX |
			X3DAUDIO_CALCULATE_DOPPLER |
			X3DAUDIO_CALCULATE_LPF_DIRECT,
			&DSPSettings);

		voice->xaudioVoice->SetOutputMatrix(m_masteringVoice, DSPSettings.SrcChannelCount, DSPSettings.DstChannelCount, DSPSettings.pMatrixCoefficients);
		voice->xaudioVoice->SetFrequencyRatio(DSPSettings.DopplerFactor);

		voice->xaudioVoice->SetVolume(sound->getVolume() * VolumeMultiplier);
	}
}

void XAudio2Driver::play(PlayingSound::ptr& sound, HgTime startOffset)
{
	auto v = initVoice(sound);
	v->submitAudio();

	startVoicePlaying(*v);
	InsertVoice(v);
}

void XAudio2Driver::startVoicePlaying(Voice& v)
{
	auto hr = v.xaudioVoice->Start(0);
	if (hr != S_OK)
	{
		std::cerr << "Failed to start playing sound: " << hr << std::endl;
	}
}

static void xaudioVoiceDeleter(IXAudio2SourceVoice* p)
{
	p->DestroyVoice();
}

std::unique_ptr<Voice> XAudio2Driver::initVoice(PlayingSound::ptr& sound)
{
	auto v = std::make_unique<Voice>();

	auto asset = sound->getSoundAsset();
	auto info = sound->getSourceInfo();

	WAVEFORMATEX format;
	format.cbSize = 0;
	format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	format.nChannels = info.channels;
	format.nSamplesPerSec = info.sampleRate;
	format.nAvgBytesPerSec = 4 * format.nChannels * format.nSamplesPerSec;
	format.wBitsPerSample = 32;
	format.nBlockAlign = (format.nChannels*format.wBitsPerSample) / 8;

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

	v->xaudioVoice = pSourceVoice;
	v->callback = voiceCallback;
	v->sound = sound;

	return v;
}

Voice::~Voice()
{
	xaudioVoice = nullptr;
}

void Voice::submitAudio()
{
	XAUDIO2_BUFFER buffer;
	memset(&buffer, 0, sizeof(XAUDIO2_BUFFER));

	auto asset = sound->getSoundAsset();

	//move this to callback logic?
	const auto packet = sound->getAudioSamples();
	buffer.AudioBytes = packet.ByteCount();
	buffer.pAudioData = reinterpret_cast<const BYTE*>(packet.audioSamples);

	// tell the source voice not to expect any data after this buffer
	buffer.Flags = packet.hasMorePackets ? 0 : XAUDIO2_END_OF_STREAM;
	buffer.pContext = this;

	hasMoreSamples = packet.hasMorePackets;

	//	auto sourceVoice = std::shared_ptr<IXAudio2SourceVoice>(pSourceVoice, xaudioVoiceDeleter);

	HRESULT hr = xaudioVoice->SubmitSourceBuffer(&buffer);
	if (hr != S_OK)
	{
		std::cerr << "Failed to submit XAUDIO2_BUFFER (" << hr << ")" << std::endl;
	}

	xaudioVoice->SetVolume(sound->getVolume() * VolumeMultiplier);
}

void XAudio2Driver::play3d(PlayingSound::ptr& sound, const Emitter& emitter)
{
	auto asset = sound->getSoundAsset();
	auto info = sound->getSourceInfo();
	if (info.channels > 1)
	{
		std::cerr << "Can not play multichannel sound in 3D. Mono Only. " << std::endl;
		return;
	}

	auto v = initVoice(sound);
	v->submitAudio();

	X3DAUDIO_EMITTER x_emitter = { 0 };
	x_emitter.ChannelCount = info.channels;
	x_emitter.CurveDistanceScaler = 30.0f;

	X3DAUDIO_DSP_SETTINGS DSPSettings = { 0 };
	DSPSettings.SrcChannelCount = info.channels; //number emitter channels
	DSPSettings.DstChannelCount = m_masteringVoiceDetails.InputChannels; //destination voice channels
	std::unique_ptr<FLOAT32[]> matrix(new FLOAT32[DSPSettings.SrcChannelCount*DSPSettings.DstChannelCount]); //SrcChannelCount * DstChannelCount
	DSPSettings.pMatrixCoefficients = matrix.get();

	updateEmitter(emitter, x_emitter);

	X3DAUDIO_LISTENER x_listener;
	memset(&x_listener, 0, sizeof(X3DAUDIO_LISTENER));
	updateListener(getListener(), x_listener);

	X3DAudioCalculate(m_xaudio3d, &x_listener, &x_emitter,
		X3DAUDIO_CALCULATE_MATRIX |
		X3DAUDIO_CALCULATE_DOPPLER |
		X3DAUDIO_CALCULATE_LPF_DIRECT,
		&DSPSettings);

	v->xaudioVoice->SetOutputMatrix(m_masteringVoice, DSPSettings.SrcChannelCount, DSPSettings.DstChannelCount, DSPSettings.pMatrixCoefficients);
	v->xaudioVoice->SetFrequencyRatio(DSPSettings.DopplerFactor);
	//pSFXSourceVoice->SetOutputMatrix(pSubmixVoice, 1, 1, &DSPSettings.ReverbLevel);

	startVoicePlaying(*v);

	auto v3d = std::make_shared<Voice3D>();
	v3d->dsp_settings = DSPSettings;
	v3d->emitter = x_emitter;
	v3d->pMatrixCoefficients = matrix.release();
	v->voice3d = v3d;

	sound->setEmitter(emitter);

	InsertVoice(v);
}

void XAudio2Driver::InsertVoice(std::unique_ptr<Voice>& v)
{
	std::lock_guard<std::mutex> lock(m_callbackMtx);

	if (v->voice3d)
	{
		m_3dvoices.push_back(std::move(v));
	}
	else
	{
		m_voices.push_back(std::move(v));
	}
}

static std::unique_ptr<Voice> removeVoice(std::vector<std::unique_ptr<Voice>>& voices, VoiceCallback* x)
{
	std::unique_ptr<Voice> ret;

	auto itr = std::find_if(voices.begin(), voices.end(), [x](std::unique_ptr<Voice>& p)
	{
		return *p == x;
	});

	if (itr != voices.end())
	{
		ret = std::move(*itr);
		*itr = std::move(voices.back());
		//		std::iter_swap(itr, voices.back());
		voices.pop_back();
	}

	return std::move(ret);
}

std::unique_ptr<Voice> XAudio2Driver::RemoveVoice(VoiceCallback* x)
{
	std::lock_guard<std::mutex> lock(m_callbackMtx);

	std::unique_ptr<Voice> ret = removeVoice(m_voices, x);

	if (ret)
	{
		return std::move(ret);
	}

	ret = removeVoice(m_3dvoices, x);

	return std::move(ret);
}

static void _stopPlayback(std::vector<std::unique_ptr<Voice>>& voices, const PlayingSound* sound)
{
	auto itr = std::find_if(voices.begin(), voices.end(), [sound](std::unique_ptr<Voice>& p)
	{
		return *p == sound;
	});

	if (itr != voices.end())
	{
		auto& voice = *itr;
		//		voice->xaudioVoice->Stop();
		voice->xaudioVoice->DestroyVoice();
		*itr = std::move(voices.back());
		voices.pop_back();
	}
}


void XAudio2Driver::stopPlayback(const PlayingSound* sound)
{
	std::lock_guard<std::mutex> lock(m_callbackMtx);

	_stopPlayback(m_voices, sound);
	_stopPlayback(m_3dvoices, sound);
}

void XAudio2Driver::update()
{
	std::lock_guard<std::mutex> lock(m_callbackMtx);

	for (auto& voice : m_3dvoices)
	{
		auto entity = voice->sound->getEmittingEntity();
		if (entity.isValid())
		{
			Emitter tmp = voice->sound->getEmitter();
			tmp.setPosition( entity->computeWorldSpacePosition() );
			voice->sound->setEmitter(tmp);
		}
	}
}

void VoiceCallback::OnStreamEnd()
{
	//std::cout << "stream ended" << std::endl;
	soundDriver->queueDestroy(this);
}

void VoiceCallback::OnBufferEnd(void * pBufferContext)
{
	Voice* v = (Voice*)pBufferContext;

	if (v->hasMoreSamples)
	{
		v->submitAudio();
	}
}

}} //namespaces