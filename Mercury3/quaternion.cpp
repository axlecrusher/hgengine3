#include <quaternion.h>
#include <HgMath.h>

#include <memory>
#include <cmath>
#include <HgTypes.h>

const quaternion quaternion_default = { 1.0f,0,0,0 };

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

	return tmp.normal();
}

//should be vector, but vector include is broken
quaternion getRotationTo(const vertex3d& v1, const vertex3d& v2) {
	//https://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another
	quaternion q = vector3_to_quat(v1.cross(v2));
	float x = v1.dot(v2);
	float y = v1.squaredLength() * v2.squaredLength();
	q.w(x+y);
	return q.normal();
}

quaternion vector3_to_quat(const vertex3d& a) {
	return quaternion(1.0, a.x(), a.y(), a.z());
}
