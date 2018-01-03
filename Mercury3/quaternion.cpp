#include <quaternion.h>
#include <HgMath.h>

#include <memory>

const quaternion quaternion_default = { 1.0f,0,0,0 };

quaternion::quaternion()
{
	wxyz = { 1.0f, 0, 0, 0 };
}

quaternion::quaternion(float w, float x, float y, float z)
{
	wxyz = { w, x, y, z };
}

quaternion::quaternion(const quaternion& lhs)
{
	wxyz = lhs.wxyz;
}

quaternion quaternion::invert() const {
	return quaternion(-w(), x(), y(), z());
}

void toQuaternion(double x, double y, double z, double deg, quaternion* q)
{
	// don't know how to properly use this function...
	double rad = deg * RADIANS;
//	x *= RADIANS;
//	y *= RADIANS;
//	z *= RADIANS;

	double s_rad = sin(rad*0.5);
	q->w( (float)cos(rad*0.5) );
	q->x(  (float)(s_rad * cos(x)));
	q->y(  (float)(s_rad * cos(y)));
	q->z(  (float)(s_rad * cos(z)));
}

quaternion toQuaternion2(double pitch, double roll, double yaw) {

	double y = yaw * RADIANS;
	double r = roll * RADIANS;
	double p = pitch * RADIANS;

	double t0 = cos(y * 0.5);
	double t1 = sin(y * 0.5);
	double t2 = cos(r * 0.5);
	double t3 = sin(r * 0.5);
	double t4 = cos(p * 0.5);
	double t5 = sin(p * 0.5);

	quaternion tmp;
	tmp.w ( (float)(t0 * t2 * t4 + t1 * t3 * t5) );
	tmp.x ( (float)(t0 * t3 * t4 - t1 * t2 * t5) );
	tmp.y ( (float)(t0 * t2 * t5 + t1 * t3 * t4) );
	tmp.z ( (float)(t1 * t2 * t4 - t0 * t3 * t5) );

	return quaternion_normalize(tmp);
}

#define SQUARE(x) (x*x)

float quat_length(quaternion* q) {
	return (float)sqrt(SQUARE(q->x()) + SQUARE(q->y()) + SQUARE(q->z()) + SQUARE(q->w()));
}

quaternion quaternion_normalize(quaternion q) {
	quaternion r = q;
	float l = (float)sqrt( SQUARE(q.x()) + SQUARE(q.y()) + SQUARE(q.z()) + SQUARE(q.w()) );
	r.w ( q.w() / l );
	r.x ( q.x() / l );
	r.y ( q.y() / l );
	r.z ( q.z() / l );
	return std::move(r);
}

quaternion quat_mult(const quaternion* q, const quaternion* r) {
	quaternion t;
	t.w ( (r->w()*q->w()) - (r->x()*q->x()) - (r->y()*q->y()) - (r->z()*q->z()) );
	t.x ( (r->w()*q->x()) + (r->x()*q->w()) - (r->y()*q->z()) + (r->z()*q->y()) );
	t.y ( (r->w()*q->y()) + (r->x()*q->z()) + (r->y()*q->w()) - (r->z()*q->x()) );
	t.z ( (r->w()*q->z()) - (r->x()*q->y()) + (r->y()*q->x()) + (r->z()*q->w()) );
//	quaternion_normal(&t);
	return std::move(t);
}

quaternion vector3_to_quat(vector3 a) {
	return quaternion(1.0, a.components.x, a.components.y, a.components.z);
}


vector3 vector3_quat_rotate(vector3 v, const quaternion& q) {
	vector3 r1 = vector3_scale(v, q.w());
	vector3 r2 = vector3_cross((vector3*)&q.wxyz.wxyz[1], &v); //XXX fix this
	r1 = vector3_add(&r1, &r2);
	r2 = vector3_cross((vector3*)&q.wxyz.wxyz[1], &r1); //XXX fix this
	r1 = vector3_scale(r2, 2.0);
	r2 = vector3_add(&v, &r1);
	return r2;
}