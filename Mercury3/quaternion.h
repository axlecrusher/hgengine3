#pragma once

#include <cmath>
#include <HgMath.h>

#include <xmmintrin.h>

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

	inline float squaredLength() const {
		return HgMath::square(x()) + HgMath::square(y()) + HgMath::square(z()) + HgMath::square(w());
	}

	inline float length() const {
		return (float)sqrt(squaredLength());
	}

	inline quaternion operator*(const quaternion& rhs) {
		return quat_mult(this, &rhs).normal();
	}

	inline quaternion normal() const {
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

private:
	inline quaternion quat_mult(const quaternion* q, const quaternion* r) {
		quaternion t;
		t.w((r->w()*q->w()) - (r->x()*q->x()) - (r->y()*q->y()) - (r->z()*q->z()));
		t.x((r->w()*q->x()) + (r->x()*q->w()) - (r->y()*q->z()) + (r->z()*q->y()));
		t.y((r->w()*q->y()) + (r->x()*q->z()) + (r->y()*q->w()) - (r->z()*q->x()));
		t.z((r->w()*q->z()) - (r->x()*q->y()) + (r->y()*q->x()) + (r->z()*q->w()));
		return t; //rvo
	}
};

extern const quaternion quaternion_default;
//inline void quaternion_init(quaternion* q) { *q = quaternion_default;  }

void toQuaternion(double x, double y, double z, double deg, quaternion* q);
//quaternion toQuaternion2(double pitch, double roll, double yaw);

class vertex3d;
quaternion vector3_to_quat(const vertex3d& a);

//Produces shortest arc from v1 to v2
quaternion getRotationTo(const vertex3d& v1, const vertex3d& v2);

//vector3 vector3_quat_rotate(const vector3& v, const quaternion& q);
