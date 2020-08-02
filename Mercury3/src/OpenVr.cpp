#include <OpenVr.h>
#include <EventSystem.h>
#include <Logging.h>

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
	LOG_ERROR(buf);
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
			LOG_ERROR("Failed to start openvr: %d", error);
			printError("Unable to init VR runtime : %s" ,error);
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


void OpenVrProxy::HandleInput()
{
	// Process SteamVR events
	vr::VREvent_t event;
	while (m_HMD->PollNextEvent(&event, sizeof(event)))
	{
		if (event.eventType == vr::EVREventType::VREvent_ButtonPress)
		{
			auto button = event.data.controller.button;
			if (button == vr::EVRButtonId::k_EButton_SteamVR_Trigger)
			{
				auto trigger = Events::VrControllerButton();
				trigger.controllerId = event.trackedDeviceIndex;
				trigger.isDown = true;
				trigger.buttonType = Events::ButtonType::BUTTON_TRIGGER;
				EventSystem::PublishEvent(trigger);
				//printf("trigger down\n");
			}
		}
		if (event.eventType == vr::EVREventType::VREvent_ButtonUnpress)
		{
			auto button = event.data.controller.button;
			if (button == vr::EVRButtonId::k_EButton_SteamVR_Trigger)
			{
				auto trigger = Events::VrControllerButton();
				trigger.controllerId = event.trackedDeviceIndex;
				trigger.isDown = false;
				trigger.buttonType = Events::ButtonType::BUTTON_TRIGGER;
				EventSystem::PublishEvent(trigger);
				//printf("trigger up\n");
			}
		}
//		ProcessVREvent(event);
	}

	vr::VRActiveActionSet_t actionSet = { 0 };
	//actionSet.ulActionSet = m_actionsetDemo;
	vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);
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
	quaternion q;

	q.w( sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2 );
	q.x( sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2 );
	q.y( sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2 );
	q.z( sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2 );
	q.x( copysign(q.x(), matrix.m[2][1] - matrix.m[1][2]) );
	q.y( copysign(q.y(), matrix.m[0][2] - matrix.m[2][0]) );
	q.z( copysign(q.z(), matrix.m[1][0] - matrix.m[0][1]) );

	return q;
}

std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t deviceIndex, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *error)
{
	std::string str;
	uint32_t bufferSize = vr::VRSystem()->GetStringTrackedDeviceProperty(deviceIndex, prop, NULL, 0, error);
	if (bufferSize > 0)
	{
		auto buffer = std::unique_ptr<char[]>(new char[bufferSize]);
		auto r = vr::VRSystem()->GetStringTrackedDeviceProperty(deviceIndex, prop, buffer.get(), bufferSize, error);
		str = buffer.get();
	}
	return str;
}

void ConstructPositionOrientationFromVrDevice(const vr::HmdMatrix34_t& mat, point& position, quaternion& orientation)
{
	orientation = GetRotation(mat);
	position = point(mat.m[0][3], mat.m[1][3], mat.m[2][3]);
}

HgCamera ConstructCameraFromVrDevice(const vr::HmdMatrix34_t& mat)
{
	HgCamera c;

	quaternion orientation;
	point position;

	ConstructPositionOrientationFromVrDevice(mat, position, orientation);

	c.setWorldSpacePosition(position);
	c.setWorldSpaceOrientation(orientation);

	return c;
}


REGISTER_EVENT_TYPE(Events::VrControllerButton);