#include <quaternion.h>
#include <HgMath.h>

void toQuaternion(double x, double y, double z, double deg, quaternion* q)
{
	// don't know how to properly use this function...
	double rad = deg * RADIANS;
//	x *= RADIANS;
//	y *= RADIANS;
//	z *= RADIANS;

	double s_rad = sin(rad*0.5);
	q->w = (float)cos(rad*0.5);
	q->x = (float)(s_rad * cos(x));
	q->y = (float)(s_rad * cos(y));
	q->z = (float)(s_rad * cos(z));
}

void toQuaternion2(double pitch, double roll, double yaw, quaternion* q) {

	double y = yaw * RADIANS;
	double r = roll * RADIANS;
	double p = pitch * RADIANS;

	double t0 = cos(y * 0.5);
	double t1 = sin(y * 0.5);
	double t2 = cos(r * 0.5);
	double t3 = sin(r * 0.5);
	double t4 = cos(p * 0.5);
	double t5 = sin(p * 0.5);

	q->w = (float)(t0 * t2 * t4 + t1 * t3 * t5);
	q->x = (float)(t0 * t3 * t4 - t1 * t2 * t5);
	q->y = (float)(t0 * t2 * t5 + t1 * t3 * t4);
	q->z = (float)(t1 * t2 * t4 - t0 * t3 * t5);

	quaternion_normalize(q);
}

#define SQUARE(x) (x*x)

void quaternion_normalize(quaternion* q) {
	float l = sqrt( SQUARE(q->x) + SQUARE(q->y) + SQUARE(q->z) + SQUARE(q->w) );
	q->w = q->w / l;
	q->x = q->x / l;
	q->y = q->y / l;
	q->z = q->z / l;
}

quaternion quat_mult(const quaternion* q, const quaternion* r) {
	quaternion t;
	t.w = (r->w*q->w) - (r->x*q->x) - (r->y*q->y) - (r->z*q->z);
	t.x = (r->w*q->x) + (r->x*q->w) - (r->y*q->z) + (r->z*q->y);
	t.y = (r->w*q->y) + (r->x*q->z) + (r->y*q->w) - (r->z*q->x);
	t.z = (r->w*q->z) - (r->x*q->y) + (r->y*q->x) + (r->z*q->w);
//	quaternion_normal(&t);
	return t;
}
