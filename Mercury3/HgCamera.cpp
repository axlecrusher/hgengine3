#include <HgCamera.h>
#include <HgMath.h>

vector3 HgCamera::projectRay() {
	vector3 ray = { 0, 0, -1 };
	quaternion q = rotation;
	q.w = -q.w;
	ray = vector3_quat_rotate(&ray, &q);
	return vector3_normalize(&ray);
}