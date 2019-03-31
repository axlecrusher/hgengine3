#pragma once

#include <openvr/openvr.h>
/* This file should only be included once in the main cpp file of the program.*/

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
private:
	vr::IVRSystem* m_HMD;
};


HgMath::mat4f ConvertToMat4f(const vr::HmdMatrix34_t &mat);
HgMath::mat4f ConvertToMat4f(const vr::HmdMatrix44_t &mat);
quaternion GetRotation(const vr::HmdMatrix34_t& matrix);
HgCamera ConstructCameraFromVrDevice(vr::HmdMatrix34_t& mat);