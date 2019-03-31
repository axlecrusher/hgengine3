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

void OpenVrProxy::printError(char* str, vr::HmdError error)
{
	char buf[1024];
	sprintf_s(buf, sizeof(buf), str, vr::VR_GetVRInitErrorAsEnglishDescription(error));
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
			fprintf(stderr, "Failed to start openvr: %d\n", error);
			printError("Unable to init VR runtime : %s\n" ,error);
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


HgMath::mat4f ConvertToMat4f(const vr::HmdMatrix34_t &mat)
{
	const vectorial::vec4f c1(mat.m[0]);
	const vectorial::vec4f c2(mat.m[1]);
	const vectorial::vec4f c3(mat.m[2]);
	const vectorial::vec4f c4(0, 0, 0, 1);

	auto r = HgMath::mat4f(c1, c2, c3, c4);
	return r;
}

HgMath::mat4f ConvertToMat4f(const vr::HmdMatrix44_t &mat)
{
	const vectorial::vec4f c1(mat.m[0]);
	const vectorial::vec4f c2(mat.m[1]);
	const vectorial::vec4f c3(mat.m[2]);
	const vectorial::vec4f c4(mat.m[3]);

	return HgMath::mat4f(c1, c2, c3, c4);
}


quaternion GetRotation(const vr::HmdMatrix34_t& matrix) {
	vr::HmdQuaternion_t q;

	q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
	q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
	q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);

	const quaternion rq(q.w, q.x, q.y, q.z);
	return rq;
}

HgCamera ConstructCameraFromVrDevice(vr::HmdMatrix34_t& mat)
{
	const auto orientation = GetRotation(mat);
	const point position(mat.m[0][3], mat.m[1][3], mat.m[2][3]);

	HgCamera c;
	c.setWorldSpacePosition(position);
	c.setWorldSpaceOrientation(orientation);

	return c;
}