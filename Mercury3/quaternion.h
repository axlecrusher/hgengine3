#pragma once

#include <math.h>
#include <HgTypes.h>

#include <xmmintrin.h>

template<typename T>
T square(T x) { return x*x; }

class quaternion {
public:
	quaternion();
	quaternion(const quaternion& lhs);
	quaternion(float w,float x, float y, float z);

	static quaternion fromEuler(double x, double y, double z);

	inline quaternion invert() const { return quaternion(w(), -x(), -y(), -z()); }

	inline float w() const { return wxyz.wxyz[0]; }
	inline void w(float a) { wxyz.wxyz[0] = a; }

	inline float x() const { return wxyz.wxyz[1]; }
	inline void x(float a) { wxyz.wxyz[1] = a; }

	inline float y() const { return wxyz.wxyz[2]; }
	inline void y(float a) { wxyz.wxyz[2] = a; }

	inline float z() const { return wxyz.wxyz[3]; }
	inline void z(float a) { wxyz.wxyz[3] = a; }

	inline float length() const {
		return (float)sqrt(square(x()) + square(y()) + square(z()) + square(w()));
	}

	inline quaternion normalize() const {
		quaternion r;
		float l = length();
		r.w(w() / l);
		r.x(x() / l);
		r.y(y() / l);
		r.z(z() / l);
		return r;
	}

	union {
		float wxyz[4];
//		__m128 sse_data; //using this and an enhanced instruction set (SSE2) seem to make the compiler do the correct things SOMETIMES rather than never
	} wxyz;
};

extern const quaternion quaternion_default;
inline void quaternion_init(quaternion* q) { *q = quaternion_default;  }

void toQuaternion(double x, double y, double z, double deg, quaternion* q);
//quaternion toQuaternion2(double pitch, double roll, double yaw);

quaternion quat_mult(const quaternion* q1, const quaternion* q2);
inline quaternion quat_mult(const quaternion& q, const quaternion& r) { return quat_mult(&q, &r); }

quaternion vector3_to_quat(vector3 a);
vector3 vector3_quat_rotate(vector3 v, const quaternion& q);
