#include <HgCamera.h>
#include <HgMath.h>
#include <math/matrix.h>

HgCamera::HgCamera() : initialDirection({0,0,-1}), orientation(quaternion::IDENTITY)
{
}

void HgCamera::FreeRotate(HgMath::angle yaw, HgMath::angle pitch) {
	using namespace HgMath;
	//seperate quaternions keep the camera from rolling when yawing and pitching
	const auto p = quaternion::fromEuler(pitch, angle::ZERO, angle::ZERO);
	const auto y = quaternion::fromEuler(angle::ZERO, yaw, angle::ZERO);
	const auto r = orientation * p;
	setWorldSpaceOrientation((y*r).normal());
}

// Project a ray in the direction of the camera's view
vector3 HgCamera::getForward() const {
	const vector3 forward(0, 0, -1); //world forward
	const vector3 ray = forward.rotate(getWorldSpaceOrientation());
	return ray.normal();
}

vector3 HgCamera::getUp() const {
	const vector3 up(0, 1, 0); //world up
	const vector3 ray = up.rotate(getWorldSpaceOrientation());
	return ray.normal();
}

HgMath::mat4f HgCamera::toViewMatrix() const {
	using namespace HgMath;

	mat4f ret;
	const vectorial::vec3f translation(-position.x(), -position.y(), -position.z());
	ret = orientation.conjugate().toMatrix4() * mat4f::translation(translation);

	return ret;
}
