#pragma once

#include <IRenderTarget.h>
#include <IFramebuffer.h>
#include <OpenVr.h>

#include <RenderBackend.h> //for viewport

class OpenVRRenderTarget : public IRenderTarget
{
public:
	OpenVRRenderTarget(OpenVrProxy* openvr);

	~OpenVRRenderTarget()
	{}

	virtual bool Init();
	virtual void Render(HgCamera* camera, RenderQueue* queue);

	void updateHMD();

	inline HgTime deltaTime() const { return m_timeSinceLastPose; }
	inline HgTime poseTime() const { return m_timeOfPose; }

private:
	void initEyeFramebuffers();

	HgMath::mat4f getHMDProjectionEye(vr::EVREye eye);
	HgMath::mat4f getHMDPoseEye(vr::EVREye eye);

	bool m_initalized;

//	OpenVrProxy m_openvr;
	OpenVrProxy* m_openvr;

	std::unique_ptr<IFramebuffer> m_leftEye;
	std::unique_ptr<IFramebuffer> m_rightEye;

	HgCamera m_hmdCamera; //camera construct from HMD pose


	HgMath::mat4f m_projection;

	HgMath::mat4f m_HMDPose;
	HgMath::mat4f m_projectionLeftEye;
	HgMath::mat4f m_projectionRightEye;
	HgMath::mat4f m_leftEyePos;
	HgMath::mat4f m_rightEyePos;

	Viewport m_windowViewport;
	Viewport m_framebufferViewport;

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	vr::ETrackedDeviceClass m_deviceClass[vr::k_unMaxTrackedDeviceCount];

	bool m_timerInited;
	HgTimer m_timer;
	HgTime m_timeSinceLastPose, m_timeOfPose;
};

namespace Events
{

struct HMDPoseUpdated
{
	point position;
	quaternion orientation;
};

struct VrControllerPoseUpdated
{
	point position;
	quaternion orientation;
	uint16_t deviceIndex;
};

}