#include <HgCamera.h>
#include <HgMath.h>

vector3 ray_from_camera(HgCamera* c) {
	vector3 ray = { 0, 0, -1 };
	quaternion q = c->rotation;
	q.w = -q.w;
	ray = vector3_quat_rotate(&ray, &q);
	return vector3_normalize(&ray);
}