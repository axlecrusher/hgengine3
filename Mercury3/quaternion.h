#pragma once

#include <cmath>
#include <HgMath.h>

#include <xmmintrin.h>
#include <vertex3d.h>

#include <math/vector.h>
//class vertex3f;
typedef vertex3f vector3;

class quaternion {
public:
	static const quaternion IDENTITY;

	quaternion() { wxyz = { 1.0f, 0, 0, 0 }; }
	quaternion(float w, float x, float y, float z) { wxyz = { w, x, y, z }; }
	quaternion(const quaternion& lhs) { wxyz = lhs.wxyz; }

	static quaternion fromEuler(HgMath::angle x, HgMath::angle y, HgMath::angle z);

	inline quaternion conjugate() const {
		//quaternion r;
		//r.wxyz = wxyz*HgMath::vec4f(0.0, -1.0, -1.0, -1.0);
		return quaternion(w(),-x(),-y(),-z());
	}

	inline quaternion inverse() const {
		return this->normal().conjugate();
	}

	inline float w() const { return wxyz[0]; }
	inline void w(float a) { wxyz[0] = a; }

	inline float x() const { return wxyz[1]; }
	inline void x(float a) { wxyz[1] = a; }

	inline float y() const { return wxyz[2]; }
	inline void y(float a) { wxyz[2] = a; }

	inline float z() const { return wxyz[3]; }
	inline void z(float a) { wxyz[3] = a; }

	inline float squaredLength() const {
		return wxyz.squaredLength();
	}

	inline float dot(const quaternion& rhs) const {
		//if the angular difference is wanted between the quaternions,
		//normalize before calling dot()
		return wxyz.dot(rhs.wxyz);
	}

	inline quaternion operator+(const quaternion& rhs) const {
		quaternion r;
		r.wxyz = wxyz + rhs.wxyz;
		return r;
	}

	inline quaternion operator-(const quaternion& rhs) const {
		quaternion r;
		r.wxyz = wxyz - rhs.wxyz;
		return r;
	}

	//inline quaternion operator^(const quaternion& rhs) const {
	//	quaternion r;
	//	r.wxyz = wxyz ^ rhs.wxyz;
	//	return r;
	//}

	inline float magnitude() const {
		return wxyz.magnitude();
	}

	inline quaternion operator*(const quaternion& rhs) const {
		//return quat_mult(*this, rhs).normal();
		return mult(*this, rhs).normal();
	}

	inline quaternion normal() const {
		quaternion r;
		r.wxyz = wxyz.normal();
		return r;
	}

	inline quaternion scale(float m) const {
		quaternion r;
		r.wxyz = wxyz.scale(m);
		return r;
	}

	static quaternion fromAxisAngle(const vector3& axis, HgMath::angle angle);

	inline const float* raw() const { return wxyz.raw(); }

	inline static quaternion mult(const quaternion& q, const quaternion& r) {
		return mult_vectorized(q, r);
	}

	static quaternion mult_c(const quaternion& q, const quaternion& r);
	static quaternion mult_vectorized(const quaternion& q, const quaternion& r);

private:

	HgMath::vec4f wxyz;
};

//void toQuaternion(double x, double y, double z, double deg, quaternion* q);
//quaternion toQuaternion2(double pitch, double roll, double yaw);

//class vertex3f;
quaternion vector3_to_quat(const vertex3f& a);

//Produces shortest arc from v1 to v2
quaternion getRotationTo(const vertex3f& v1, const vertex3f& v2);

//vector3 vector3_quat_rotate(const vector3& v, const quaternion& q);
