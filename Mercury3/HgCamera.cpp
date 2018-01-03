#include <HgCamera.h>
#include <HgMath.h>

// Project a ray in the direction of the camera's view
vector3 HgCamera::projectRay() {
	vector3 ray = { 0, 0, -1 };
	ray = vector3_quat_rotate(ray, rotation.invert());
	return vector3_normalize(ray);
}