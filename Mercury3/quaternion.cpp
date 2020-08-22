#include <quaternion.h>
#include <HgMath.h>

#include <memory>
#include <cmath>
#include <HgTypes.h>

#include <math/vector.h>
#include <math/MatrixMath.h>

const quaternion quaternion::IDENTITY = { 1.0f,0,0,0 };

//void toQuaternion(double x, double y, double z, double deg, quaternion* q)
//{
//	// AxisAngle to Quaternion
//	// don't know how to properly use this function...
//	double rad = deg * RADIANS;
////	x *= RADIANS;
////	y *= RADIANS;
////	z *= RADIANS;
//
//	double s_rad = sin(rad*0.5);
//	q->w( (float)cos(rad*0.5) );
//	q->x(  (float)(s_rad * cos(x)));
//	q->y(  (float)(s_rad * cos(y)));
//	q->z(  (float)(s_rad * cos(z)));
//}

quaternion quaternion::fromAxisAngle(const vector3& axis, HgMath::angle angle) {
	quaternion q;

	vector3 a = axis.normal();
	double s = sin(angle.rad()*0.5);

	q.w(cos(angle.rad() * 0.5));
	q.x(a.x()*s);
	q.y(a.y()*s);
	q.z(a.z()*s);

	return q;
}

quaternion quaternion::fromEuler(HgMath::angle x, HgMath::angle y, HgMath::angle z) {
	//YZX
	//match http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/index.htm
	//double h = y.rad(); //heading
	//double a = z.rad(); //attitude
	//double b = x.rad(); //bank

	quaternion tmp;

	const double c1 = cos(y.rad() * 0.5);
	const double s1 = sin(y.rad() * 0.5);
	const double c2 = cos(z.rad() * 0.5);
	const double s2 = sin(z.rad() * 0.5);
	const double c3 = cos(x.rad() * 0.5);
	const double s3 = sin(x.rad() * 0.5);

	tmp.w((float)(c1 * c2 * c3 - s1 * s2 * s3));
	tmp.x((float)(s1 * s2 * c3 + c1 * c2 * s3));
	tmp.y((float)(s1 * c2 * c3 + c1 * s2 * s3));
	tmp.z((float)(c1 * s2 * c3 - s1 * c2 * s3));

	//result is already normalized
	return tmp;
}

//should be vector, but vector include is broken
quaternion getRotationTo(const vertex3f& v1, const vertex3f& v2) {
	//https://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another
	const float d = v1.dot(v2);

	//adapted from ogre https://bitbucket.org/sinbad/ogre/src/9db75e3ba05c0f710998b0816484f3112d5d29ed/OgreMain/include/OgreVector3.h?at=default&fileviewer=file-view-default#OgreVector3.h-664
	if (d >= 1.0f) {
		return quaternion::IDENTITY;
	}

	if (d < (1e-6f - 1.0f)) { //-0.999999
		vector3 axis = vector3::UNIT_X.cross(v1);
		if (axis.isZeroLength())
			axis = vector3::UNIT_Y.cross(v1);
		return quaternion::fromAxisAngle(axis.normal(), HgMath::angle::rad(M_PI));
	}

	quaternion q = vector3_to_quat(v1.cross(v2));
	float y = v1.squaredLength() * v2.squaredLength();
	q.w(d + y);

	return q.normal();
}

quaternion vector3_to_quat(const vertex3f& a) {
	return quaternion(1.0, a.x(), a.y(), a.z());
}

quaternion quaternion::mult_c(const quaternion& q, const quaternion& r) {
	quaternion t;
	t.w((r.w()*q.w()) - (r.x()*q.x()) - (r.y()*q.y()) - (r.z()*q.z()));
	t.x((r.w()*q.x()) + (r.x()*q.w()) - (r.y()*q.z()) + (r.z()*q.y()));
	t.y((r.w()*q.y()) + (r.x()*q.z()) + (r.y()*q.w()) - (r.z()*q.x()));
	t.z((r.w()*q.z()) - (r.x()*q.y()) + (r.y()*q.x()) + (r.z()*q.w()));
	return t; //rvo
}

inline HgMath::vec4f compute(const HgMath::vec4f& a, const HgMath::vec4f& b) {
	return a * b;
}

quaternion quaternion::mult_vectorized(const quaternion& q, const quaternion& r) {
	using namespace HgMath;
	quaternion result;

	//maybe do one or two at a time
	const vec4f w = vec4f(r.w());
	const vec4f x = vec4f(r.x()) * vec4f(-1.0, 1.0, 1.0, -1.0);
	const vec4f y = vec4f(r.y()) * vec4f(-1.0, -1.0, 1.0, 1.0);
	const vec4f z = vec4f(r.z()) * vec4f(-1.0, 1.0, -1.0, 1.0);

	const vec4f a = q.wxyz;
	const vec4f b(q.x(), q.w(), q.z(), q.y());
	const vec4f c(q.y(), q.z(), q.w(), q.x());
	const vec4f d(q.z(), q.y(), q.x(), q.w());

	result.wxyz = w * a + x * b +y * c + z * d;

	return result; //rvo
}

void quaternion::toMatrix4(float* m) const {
	using namespace HgMath;

	//const float sx = square(x());
	//const float sy = square(y());
	//const float sz = square(z());
	const auto sq = square(wxyz);

	const float qxqy2 = 2.0f * x()*y();
	const float qzqw2 = 2.0f * z()*w();
	const float qxqz2 = 2.0f * x()*z();
	const float qyqw2 = 2.0f * y()*w();
	const float qyqz2 = 2.0f * y()*z();
	const float qxqw2 = 2.0f * x()*w();

	//column major
	m[0] = 1.0f - 2 * sq.y() -2 * sq.z();
	m[1] = qxqy2 + qzqw2;
	m[2] = qxqz2 - qyqw2;
	m[3] = 0;

	m[4] = qxqy2 - qzqw2;
	m[5] = 1.0f - 2 * sq.x() - 2 * sq.z();
	m[6] = qyqz2 + qxqw2;
	m[7] = 0;

	m[8] = qxqz2 + qyqw2;
	m[9] = qyqz2 - qxqw2;
	m[10] = 1.0f - 2 * sq.x() -2 * sq.y();
	m[11] = 0;

	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;
}

void quaternion::toMatrix4(HgMath::mat4f& m) const
{
	using namespace HgMath;

	//const float sx = square(x());
	//const float sy = square(y());
	//const float sz = square(z());
	const auto sq = square(wxyz);

	const float qxqy2 = 2.0f * x()*y();
	const float qzqw2 = 2.0f * z()*w();
	const float qxqz2 = 2.0f * x()*z();
	const float qyqw2 = 2.0f * y()*w();
	const float qyqz2 = 2.0f * y()*z();
	const float qxqw2 = 2.0f * x()*w();

	//column major

	const auto v0 = simd4f_create(
		1.0f - 2 * sq.y() - 2 * sq.z(),
		qxqy2 + qzqw2,
		qxqz2 - qyqw2,
		0.0);

	const auto v1 = simd4f_create(
		qxqy2 - qzqw2,
		1.0f - 2 * sq.x() - 2 * sq.z(),
		qyqz2 + qxqw2,
		0.0);

	const auto v2 = simd4f_create(
		qxqz2 + qyqw2,
		qyqz2 - qxqw2,
		1.0f - 2 * sq.x() - 2 * sq.y(),
		0.0);

	//const auto v3 = simd4f_create(vectorial::vec4f::wAxis());

	m.value.x = v0;
	m.value.y = v1;
	m.value.z = v2;
	m.value.w = vectorial::vec4f::wAxis().value;
}
