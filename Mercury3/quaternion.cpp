#include <quaternion.h>
#include <HgMath.h>

#include <memory>
#include <cmath>

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

void toQuaternion(double x, double y, double z, double deg, quaternion* q)
{
	// AxisAngle to Quaternion
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

//quaternion quaternion::fromEuler(double y, double x, double z) {
//
//	double y = z * RADIANS;
//	double r = x * RADIANS;
//	double p = y * RADIANS;
//
//	double c1 = cos(y * 0.5);
//	double s1 = sin(y * 0.5);
//	double c3 = cos(r * 0.5);
//	double s3 = sin(r * 0.5);
//	double c2 = cos(p * 0.5);
//	double s2 = sin(p * 0.5);
//
//	quaternion tmp;
//	tmp.w ( (float)(c1 * c3 * c2 + s1 * s3 * s2) );
//	tmp.x ( (float)(c1 * s3 * c2 - s1 * c3 * s2) );
//	tmp.y ( (float)(c1 * c3 * s2 + s1 * s3 * c2) );
//	tmp.z ( (float)(s1 * c3 * c2 - c1 * s3 * s2) );
//
//	return tmp.normalize();
//}

quaternion quaternion::fromEuler(double x, double y, double z) {
	//match http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/index.htm
	double h = y * RADIANS; //heading
	double a = z * RADIANS; //attitude
	double b = x * RADIANS; //bank

	double c1 = cos(h * 0.5);
	double s1 = sin(h * 0.5);
	double c2 = cos(a * 0.5);
	double s2 = sin(a * 0.5);
	double c3 = cos(b * 0.5);
	double s3 = sin(b * 0.5);

	quaternion tmp;
	tmp.w((float)(c1 * c2 * c3 - s1 * s2 * s3));
	tmp.x((float)(s1 * s2 * c3 + c1 * c2 * s3));
	tmp.y((float)(s1 * c2 * c3 + c1 * s2 * s3));
	tmp.z((float)(c1 * s2 * c3 - s1 * c2 * s3));

	return tmp.normalize();
}

//#define SQUARE(x) (x*x)
//
//quaternion quaternion_normalize(const quaternion& q) {
//	quaternion r = q;
//	float l = (float)sqrt( SQUARE(q.x()) + SQUARE(q.y()) + SQUARE(q.z()) + SQUARE(q.w()) );
//	r.w ( q.w() / l );
//	r.x ( q.x() / l );
//	r.y ( q.y() / l );
//	r.z ( q.z() / l );
//	return r;
//}
//
//quaternion quat_mult(const quaternion* q, const quaternion* r) {
//	quaternion t;
//	t.w ( (r->w()*q->w()) - (r->x()*q->x()) - (r->y()*q->y()) - (r->z()*q->z()) );
//	t.x ( (r->w()*q->x()) + (r->x()*q->w()) - (r->y()*q->z()) + (r->z()*q->y()) );
//	t.y ( (r->w()*q->y()) + (r->x()*q->z()) + (r->y()*q->w()) - (r->z()*q->x()) );
//	t.z ( (r->w()*q->z()) - (r->x()*q->y()) + (r->y()*q->x()) + (r->z()*q->w()) );
//	return t; //rvo
//}

quaternion vector3_to_quat(vector3 a) {
	return quaternion(1.0, a.x(), a.y(), a.z());
}


vector3 vector3_quat_rotate(vector3 v, const quaternion& q) {
	vector3 r1 = v.scale( q.w() );
	vector3 r2 = vector3(q.wxyz.wxyz + 1).cross(v); //XXX fix this
	r1 = r1 + r2;
	r2 = vector3(q.wxyz.wxyz + 1).cross(r1); //XXX fix this
	r1 = r2.scale(2.0);
	r2 = v + r1;
	return r2;
}