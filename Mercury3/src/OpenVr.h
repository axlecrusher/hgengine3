#pragma once

#include <openvr/openvr.h>

#include <quaternion.h>
#include <HgCamera.h>

class OpenVrProxy
{
public:
	OpenVrProxy();
	~OpenVrProxy();

	bool Init();
	bool InitCompositor();

	inline vr::IVRSystem* getDevice() const { return m_HMD; }
	static void printError(char* str, vr::HmdError error);

	inline bool isValid() const { return m_HMD != nullptr; }
	void HandleInput();

private:
	vr::IVRSystem* m_HMD;
};

namespace Events
{

enum ButtonType
{
	BUTTON_INVALID = 0,
	BUTTON_TRIGGER
};

struct VrControllerButton{
	uint32_t controllerId;
	bool isDown;
	ButtonType buttonType;
};

}

std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t deviceIndex, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *error = nullptr);

HgMath::mat4f ConvertToMat4f(const vr::HmdMatrix34_t &mat);
HgMath::mat4f ConvertToMat4f(const vr::HmdMatrix44_t &mat);
quaternion GetRotation(const vr::HmdMatrix34_t& matrix);
void ConstructPositionOrientationFromVrDevice(const vr::HmdMatrix34_t& mat, point& position, quaternion& orientation);
HgCamera ConstructCameraFromVrDevice(const vr::HmdMatrix34_t& mat);