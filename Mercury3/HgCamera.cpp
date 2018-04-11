#include <HgCamera.h>
#include <HgMath.h>

HgCamera::HgCamera() : initialDirection({0,0,-1}), rotation(quaternion_default), position(vertex_zero)
{
}

void HgCamera::FreeRotate(double yaw, double pitch) {
	//seperate quaternions keep the camera from rolling when yawing and pitching
	auto p = quaternion::fromEuler(pitch, 0, 0);
	auto y = quaternion::fromEuler(0, yaw, 0);
	auto r = quat_mult(&p, &rotation);
	SetRotation(quat_mult(&r, &y).normalize());
}

// Project a ray in the direction of the camera's view
vector3 HgCamera::projectRay() {
	vector3 ray = vector3_quat_rotate(initialDirection, rotation.invert());
	return vector3_normalize(ray);
}