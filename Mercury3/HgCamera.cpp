#include <HgCamera.h>
#include <HgMath.h>
#include <math/matrix.h>

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
vector3 HgCamera::projectRay() const {
	vector3 ray = initialDirection.rotate(rotation.conjugate());
	return ray.normal();
}

HgMath::mat4f HgCamera::toMatrix() const {
	using namespace HgMath;

	//float m[16];
	//const auto matrix = rotation.toMatrix4();
	//matrix.store(m);
	//m[12] = -position.x();
	//m[13] = -position.y();
	//m[14] = -position.z();

	mat4f ret;
	const vectorial::vec3f translation(-position.x(), -position.y(), -position.z());
	ret = rotation.toMatrix4() * mat4f::translation(translation);

	return ret;
}
