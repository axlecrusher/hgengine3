#include <OpenVr.h>

OpenVrProxy::OpenVrProxy()
	:m_HMD(nullptr)
{

}

OpenVrProxy::~OpenVrProxy()
{
	if (m_HMD)
	{
		vr::VR_Shutdown();
	}

	m_HMD = nullptr;
}

static void printError(vr::HmdError error)
{
	char buf[1024];
	fprintf(stderr, "Failed to start openvr: %d\n", error);

	sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(error));
	fprintf(stderr, buf);
}

bool OpenVrProxy::Init()
{
	if (vr::VR_IsHmdPresent())
	{
		vr::EVRInitError error = vr::VRInitError_None;
		m_HMD = vr::VR_Init(&error, vr::EVRApplicationType::VRApplication_Scene);

		if (error != vr::VRInitError_None)
		{
			m_HMD = nullptr;
			printError(error);
			return false;
		}

		return true;
	}
	return false;
}

bool OpenVrProxy::InitCompositor()
{
	if (m_HMD)
	{
		vr::EVRInitError peError = vr::VRInitError_None;

		if (!vr::VRCompositor())
		{
			fprintf(stderr, "Compositor initialization failed. See log file for details\n");
			return false;
		}

		return true;
	}
	return false;
}