#include <HgCamera.h>
#include <HgMath.h>

HgCamera::HgCamera() : initialDirection({0,0,-1}), rotation(quaternion::IDENTITY)
{
}

void HgCamera::FreeRotate(HgMath::angle yaw, HgMath::angle pitch) {
	using namespace HgMath;
	//seperate quaternions keep the camera from rolling when yawing and pitching
	auto p = quaternion::fromEuler(pitch, angle::ZERO, angle::ZERO);
	auto y = quaternion::fromEuler(angle::ZERO, yaw, angle::ZERO);
	auto r = p * rotation;
	SetRotation((r*y).normal());
}

// Project a ray in the direction of the camera's view
vector3 HgCamera::projectRay() {
	vector3 ray = initialDirection.rotate(rotation.invert());
	return ray.normal();
}