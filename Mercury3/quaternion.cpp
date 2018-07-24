#include <quaternion.h>
#include <HgMath.h>

#include <memory>
#include <cmath>
#include <HgTypes.h>

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

	return tmp.normal();
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
