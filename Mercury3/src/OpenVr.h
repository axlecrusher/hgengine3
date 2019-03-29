#pragma once

#include <openvr/openvr.h>
/* This file should only be included once in the main cpp file of the program.*/

class OpenVrProxy
{
public:
	OpenVrProxy();
	~OpenVrProxy();

	bool Init();
	bool InitCompositor();
private:
	vr::IVRSystem* m_HMD;
};