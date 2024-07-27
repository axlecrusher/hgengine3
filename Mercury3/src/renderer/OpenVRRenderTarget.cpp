#include <OpenVRRenderTarget.h>
#include <HgUtils.h>
#include <HgCamera.h>
#include <MercuryWindow.h>
#include <RenderBackend.h>
#include <Win32Window.h>

#include <OGLFramebuffer.h>
#include <EventSystem.h>
#include <Logging.h>

OpenVRRenderTarget::OpenVRRenderTarget(OpenVrProxy* openvr)
	:m_openvr(openvr), m_initalized(false), m_timerInited(false)
{
	m_HMDPose = vectorial::mat4f::identity();
	m_projectionLeftEye = vectorial::mat4f::identity();
	m_projectionRightEye = vectorial::mat4f::identity();
	//m_leftEyePos = vectorial::mat4f::identity();
	//m_rightEyePos = vectorial::mat4f::identity();

	memset(m_deviceClass, 0, sizeof(m_deviceClass));
}

bool OpenVRRenderTarget::Init()
{
//	if (!m_openvr->Init()) return false;

	auto strDriver = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	auto strDisplay = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	ENGINE::StartWindowSystem(MercuryWindow::Dimensions(1280, 720));

	auto window = MercuryWindow::GetCurrentWindow();

	int width = window->CurrentWidth();
	int height = window->CurrentHeight();

	float projection[16];

	m_windowViewport.width = width;
	m_windowViewport.height = height;

	double renderWidth = width;
	double renderHeight = height;
	double aspect = renderWidth / renderHeight;
	Perspective2(60, aspect, 0.03f, 100.0f, projection);
	m_projection.load(projection);

	m_openvr->InitCompositor();
	initEyeFramebuffers();

	m_projectionLeftEye = getHMDProjectionEye(vr::Eye_Left);
	m_projectionRightEye = getHMDProjectionEye(vr::Eye_Right);
	//m_leftEyePos = getHMDEyeTransform(vr::Eye_Left);
	//m_rightEyePos = getHMDEyeTransform(vr::Eye_Right);

	m_timerInited = false;
	m_initalized = true;
	return true;
}

void OpenVRRenderTarget::initEyeFramebuffers()
{
	auto hmd = m_openvr->getDevice();
	if (hmd == nullptr) return;

	uint32_t width, height;
	hmd->GetRecommendedRenderTargetSize(&width, &height);

	m_framebufferViewport.width = width;
	m_framebufferViewport.height = height;

	m_leftEye = RENDERER()->CreateFrameBuffer();
	m_leftEye->Init(width, height);

	m_rightEye = RENDERER()->CreateFrameBuffer();
	m_rightEye->Init(width, height);
}

void OpenVRRenderTarget::updateHMD()
{
	auto hmd = m_openvr->getDevice();
	if (hmd == nullptr) return;

	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	if (m_timerInited == false)
	{
		m_timerInited = true;
		m_timer.start();
		m_timeSinceLastPose = m_timeOfPose.msec(0);
	}
	else
	{
		const auto time = m_timer.getElasped();
		m_timeSinceLastPose = time - m_timeOfPose;
		m_timeOfPose = time;
	}

	for (int nDevice = 1; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			const auto pose = ConvertToMat4f(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			if (m_deviceClass[nDevice] == vr::ETrackedDeviceClass::TrackedDeviceClass_Invalid)
			{
				auto type = hmd->GetTrackedDeviceClass(nDevice);
				m_deviceClass[nDevice] = type;
			}

			if (m_deviceClass[nDevice] == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
			{
				Events::VrControllerPoseUpdated poseUpdated;
				ConstructPositionOrientationFromVrDevice(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking, poseUpdated.position, poseUpdated.orientation);
				poseUpdated.deviceIndex = nDevice;

				EventSystem::PublishEvent(poseUpdated);
			}
		}
	}

	if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		m_HMDPose = ConvertToMat4f(m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking);
		m_hmdCamera = ConstructCameraFromVrDevice(m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking);

		Events::HMDPoseUpdated poseUpdated;
		poseUpdated.position = m_hmdCamera.getWorldSpacePosition();
		poseUpdated.orientation = m_hmdCamera.getWorldSpaceOrientation();

		EventSystem::PublishEvent(poseUpdated);
	}
}

HgMath::mat4f OpenVRRenderTarget::getWindowProjectionMatrix()
{
	HgMath::mat4f projectionMatrix;

	float projection[16];

	const double width = m_windowViewport.width;
	const double height = m_windowViewport.height;
	const double aspect = width / height;

	Perspective2(60, aspect, 0.1f, 100.0f, projection);
	projectionMatrix.load(projection);

	return projectionMatrix;
}

//const RenderParamsList& l
//void OpenVRRenderTarget::Render(HgCamera* camera, RenderQueue* queue, const HgMath::mat4f& projection)
void OpenVRRenderTarget::Render(const RenderParamsList& l)
{
	if (!m_initalized) return;

	RENDERER()->Clear();
	RENDERER()->BeginFrame();
	ViewportRenderTarget vprt(&m_windowViewport);
	for (const RenderParams& i : l)
	{
		const auto hdmCamMatrix = i.camera->toViewMatrix();
		const auto projection = i.projection->getProjectionMatrix(vprt);
		Renderer::Render(m_windowViewport, hdmCamMatrix, projection, i.queue);
	}

	auto left = dynamic_cast<OGLFramebuffer*>(m_leftEye.get());
	auto right = dynamic_cast<OGLFramebuffer*>(m_rightEye.get());

	{
		left->Enable();
		RENDERER()->Clear();
		RENDERER()->BeginFrame();
		const auto eyeTtransform = getHMDEyeTransform(vr::Eye_Left);
		VrEyeRenderTarget rt(this, vr::EVREye::Eye_Left);

		for (const RenderParams& i : l)
		{
			const auto projection = i.projection->getProjectionMatrix(rt);
			RenderToEye(left, eyeTtransform, projection, i);
		}
		left->Disable();
		left->Copy();
	}

	{
		right->Enable();
		RENDERER()->Clear();
		RENDERER()->BeginFrame();
		const auto eyeTtransform = getHMDEyeTransform(vr::Eye_Right);
		VrEyeRenderTarget rt(this, vr::EVREye::Eye_Right);
		for (const RenderParams& i : l)
		{
			const auto projection = i.projection->getProjectionMatrix(rt);
			RenderToEye(left, eyeTtransform, projection, i);
		}
		right->Disable();
		right->Copy();
	}

	////updateHMD();

	//auto right = dynamic_cast<OGLFramebuffer*>(m_rightEye.get());





	////auto camMat = camera->toViewMatrix(); //replace with head matrix
	//const auto hdmCamMatrix = m_hmdCamera.toViewMatrix();
	//const auto leftEyeMat = m_leftEyePos * hdmCamMatrix;
	//const auto rightEyeMat = m_rightEyePos * hdmCamMatrix;

	//const auto leftProjection = m_projectionLeftEye * projection;
	//const auto rightProjection = m_projectionRightEye * projection;

	////render to window first
	//Renderer::Render(m_windowViewport, hdmCamMatrix, projection, queue);

	//m_leftEye->Enable();
	//RENDERER()->Clear();
	//RENDERER()->BeginFrame();
	//Renderer::Render(m_framebufferViewport, leftEyeMat, leftProjection, queue); //eye 1
	//m_leftEye->Disable();
	//left->Copy();

	//m_rightEye->Enable();
	//RENDERER()->Clear();
	//RENDERER()->BeginFrame();
	//Renderer::Render(m_framebufferViewport, rightEyeMat, rightProjection, queue); //eye 2
	//m_rightEye->Disable();
	//right->Copy();

	//auto left = dynamic_cast<OGLFramebuffer*>(m_leftEye.get());
	//auto right = dynamic_cast<OGLFramebuffer*>(m_rightEye.get());

	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)left->getResolveTextureID(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	auto e1 = vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);

	if (e1 != vr::VRInitError_None)
	{
		LOG_ERROR("Failed to submit left eye: %d", e1);
	}

	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)right->getResolveTextureID(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	auto e2 = vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

	if (e2 != vr::VRInitError_None)
	{
		LOG_ERROR("Failed to submit right eye: %d", e2);
	}
}

void OpenVRRenderTarget::RenderToEye(IFramebuffer* eyeBuffer, const HgMath::mat4f& eyePosTransform,
	const HgMath::mat4f& eyeProjection, const RenderParams& p)
{
	const auto hdmCamMatrix = p.camera->toViewMatrix();
	const auto eyeMatrix = eyePosTransform * hdmCamMatrix;
	//const auto projection = eyeProjection * (*p.projection);

	//frameBuffer->Enable();
	//RENDERER()->Clear();
	//RENDERER()->BeginFrame();
	Renderer::Render(m_framebufferViewport, eyeMatrix, eyeProjection, p.queue);
	//frameBuffer->Disable();
	//frameBuffer->Copy();
}

void OpenVRRenderTarget::Finish()
{

}

HgMath::mat4f OpenVRRenderTarget::getOrthoMatrix() const
{
	//return vectorial::transpose(HgMath::mat4f::translation(vectorial::vec3f(0, 0, 1)));
	return HgMath::mat4f::translation(vectorial::vec3f(0, 0, 1));
//	return vectorial::mat4f::identity();
}


//returns a projection matrix for the specified eye
HgMath::mat4f OpenVRRenderTarget::getHMDProjectionEye(vr::EVREye eye) const
{
	auto hmd = m_openvr->getDevice();
	if (!hmd) return HgMath::mat4f();

	const vr::HmdMatrix44_t mat = hmd->GetProjectionMatrix(eye, 0.1, 100);

	const auto r = vectorial::transpose(ConvertToMat4f(mat));
	return r;

}

HgMath::mat4f OpenVRRenderTarget::getHMDEyeTransform(vr::EVREye eye)
{
	auto hmd = m_openvr->getDevice();
	if (!hmd) return HgMath::mat4f();

	vr::HmdMatrix34_t mat = hmd->GetEyeToHeadTransform(eye);

	//inverse becasue GetEyeToHeadTransform documentations says to.
	const auto eyeMatrix = vectorial::inverse(ConvertToMat4f(mat));
	return vectorial::transpose(eyeMatrix);
}

REGISTER_EVENT_TYPE(Events::HMDPoseUpdated)
REGISTER_EVENT_TYPE(Events::VrControllerPoseUpdated)