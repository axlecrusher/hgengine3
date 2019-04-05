#include <OpenVRRenderTarget.h>
#include <HgUtils.h>
#include <HgCamera.h>
#include <MercuryWindow.h>
#include <RenderBackend.h>
#include <Win32Window.h>

#include <OGLFramebuffer.h>
#include <EventSystem.h>

#define EYE_DISTANCE -0.07f

OpenVRRenderTarget::OpenVRRenderTarget(OpenVrProxy* openvr)
	:m_openvr(openvr), m_initalized(false)
{
	m_HMDPose = vectorial::mat4f::identity();
	m_projectionLeftEye = vectorial::mat4f::identity();
	m_projectionRightEye = vectorial::mat4f::identity();
	m_leftEyePos = vectorial::mat4f::identity();
	m_rightEyePos = vectorial::mat4f::identity();
}

bool OpenVRRenderTarget::Init()
{
//	if (!m_openvr->Init()) return false;

	auto strDriver = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	auto strDisplay = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	ENGINE::StartWindowSystem(1920, 1080);

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
	m_leftEyePos = getHMDPoseEye(vr::Eye_Left);
	m_rightEyePos = getHMDPoseEye(vr::Eye_Right);

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

	for (int nDevice = 1; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			const auto pose = ConvertToMat4f(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			if (m_deviceClass[nDevice] != vr::ETrackedDeviceClass::TrackedDeviceClass_Invalid)
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

void OpenVRRenderTarget::Render(HgCamera* camera, RenderQueue* queue)
{
	if (!m_initalized) return;

	updateHMD();

	auto left = dynamic_cast<OGLFramebuffer*>(m_leftEye.get());
	auto right = dynamic_cast<OGLFramebuffer*>(m_rightEye.get());

	//auto camMat = camera->toViewMatrix(); //replace with head matrix
	const auto hdmCamMatrix = m_hmdCamera.toViewMatrix();
	const auto leftEyeMat = m_leftEyePos * hdmCamMatrix;
	const auto rightEyeMat = m_rightEyePos * hdmCamMatrix;

	//render to window first
	Renderer::Render(m_windowViewport, hdmCamMatrix, m_projection, queue);

	m_leftEye->Enable();
	RENDERER()->Clear();
	RENDERER()->BeginFrame();
	Renderer::Render(m_framebufferViewport, leftEyeMat, m_projectionLeftEye, queue); //eye 1
	m_leftEye->Disable();
	left->Copy();

	m_rightEye->Enable();
	RENDERER()->Clear();
	RENDERER()->BeginFrame();
	Renderer::Render(m_framebufferViewport, rightEyeMat, m_projectionRightEye, queue); //eye 2
	m_rightEye->Disable();
	right->Copy();

	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)left->getResolveTextureID(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	auto e1 = vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);

	if (e1 != vr::VRInitError_None)
	{
		fprintf(stderr, "Failed to submit left eye: %d\n", e1);
	}

	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)right->getResolveTextureID(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	auto e2 = vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

	if (e2 != vr::VRInitError_None)
	{
		fprintf(stderr, "Failed to submit right eye: %d\n", e2);
	}
}

HgMath::mat4f OpenVRRenderTarget::getHMDProjectionEye(vr::EVREye eye)
{
	auto hmd = m_openvr->getDevice();
	if (!hmd) return HgMath::mat4f();

	const vr::HmdMatrix44_t mat = hmd->GetProjectionMatrix(eye, 0.1, 100);

	const auto r = vectorial::transpose(ConvertToMat4f(mat));
	return r;

}

HgMath::mat4f OpenVRRenderTarget::getHMDPoseEye(vr::EVREye eye)
{
	auto hmd = m_openvr->getDevice();
	if (!hmd) return HgMath::mat4f();

	vr::HmdMatrix34_t mat = hmd->GetEyeToHeadTransform(eye);

	//unsure why this is messed up
	mat.m[0][3] *= -1.0;

	return vectorial::transpose(ConvertToMat4f(mat));
}

REGISTER_EVENT_TYPE(Events::HMDPoseUpdated)
REGISTER_EVENT_TYPE(Events::VrControllerPoseUpdated)