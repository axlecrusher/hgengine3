#pragma once

#include <IRenderTarget.h>
#include <IFramebuffer.h>
#include <OpenVr.h>

#include <RenderBackend.h> //for viewport

class VrEyeRenderTarget; //forward declare

class OpenVRRenderTarget : public IRenderTarget
{
public:
	OpenVRRenderTarget(OpenVrProxy* openvr);

	~OpenVRRenderTarget()
	{}

	virtual bool Init();
	virtual void Render(const RenderParamsList& l);
	//virtual void Render(HgCamera* camera, RenderQueue* queue, const HgMath::mat4f&);

	void updateHMD();

	inline HgTime deltaTime() const { return m_timeSinceLastPose; }
	inline HgTime poseTime() const { return m_timeOfPose; }

	//This is meaningless in vr. VR has 2 perspectives.
	virtual HgMath::mat4f getPerspectiveMatrix() const { return HgMath::mat4f::identity(); };
	virtual HgMath::mat4f getOrthoMatrix() const;

	virtual uint32_t getWidth() const { return m_framebufferViewport.width; };
	virtual uint32_t getHeight() const { return m_framebufferViewport.height; };

	virtual void Finish();

private:
	void initEyeFramebuffers();
	void RenderToEye(IFramebuffer* eyeBuffer, const HgMath::mat4f& eyeMatrix, const HgMath::mat4f& eyeProjection, const RenderParams& p);

	HgMath::mat4f getWindowProjectionMatrix();

	//returns a projection matrix for the specified eye
	HgMath::mat4f getHMDProjectionEye(vr::EVREye eye) const;

	/*	Returns the transformation matrix for the specified eye.
		This can change during runtime if the interpupillary distance changes.
	*/
	HgMath::mat4f getHMDEyeTransform(vr::EVREye eye);

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
	//HgMath::mat4f m_leftEyePos;
	//HgMath::mat4f m_rightEyePos;

	Viewport m_windowViewport;
	Viewport m_framebufferViewport;

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	vr::ETrackedDeviceClass m_deviceClass[vr::k_unMaxTrackedDeviceCount];

	bool m_timerInited;
	HgTimer m_timer;
	HgTime m_timeSinceLastPose, m_timeOfPose;

	friend VrEyeRenderTarget;
};

//make a viewport look like a render target
class VrEyeRenderTarget : public IRenderTarget
{
public:
	VrEyeRenderTarget(const OpenVRRenderTarget* rt, vr::EVREye eye)
		:m_rt(rt), m_eye(eye)
	{}

	virtual bool Init() { return true; };
	virtual void Render(const RenderParamsList& l) {};

	virtual HgMath::mat4f getPerspectiveMatrix() const
	{
		return m_rt->getHMDProjectionEye(m_eye);
	}

	virtual HgMath::mat4f getOrthoMatrix() const
	{
		const auto perspective = m_rt->getHMDProjectionEye(m_eye);
		return perspective * m_rt->getOrthoMatrix();
	}

	virtual uint32_t getWidth() const
	{
		return 0;
	}

	virtual uint32_t getHeight() const
	{
		return 0;
	}

private:
	const OpenVRRenderTarget* m_rt;
	vr::EVREye m_eye;
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