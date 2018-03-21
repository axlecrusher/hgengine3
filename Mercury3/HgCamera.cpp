#include <HgCamera.h>
#include <HgMath.h>

HgCamera::HgCamera() : initialDirection({0,0,-1}), rotation(quaternion_default), position(vertex_zero)
{
}

void HgCamera::FreeRotate(double yaw, double pitch) {
	//seperate quaternions keep the camera from rolling when yawing and pitching
	auto p = toQuaternion2(0, pitch, 0);
	auto y = toQuaternion2(yaw, 0, 0);
	auto r = quat_mult(&p, &rotation);
	auto final_rot = quat_mult(&r, &y);
	//normalize quaternion?
	SetRotation(final_rot);
}

// Project a ray in the direction of the camera's view
vector3 HgCamera::projectRay() {
	vector3 ray = vector3_quat_rotate(initialDirection, rotation.invert());
	return vector3_normalize(ray);
}