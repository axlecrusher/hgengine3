#pragma once

#include <math.h>
#include <HgTypes.h>

#include <xmmintrin.h>

class quaternion {
public:
	quaternion();
	quaternion(const quaternion& lhs);
	quaternion(float w,float x, float y, float z);
	quaternion invert() const;

	inline float w() const { return wxyz.wxyz[0]; }
	inline void w(float a) { wxyz.wxyz[0] = a; }

	inline float x() const { return wxyz.wxyz[1]; }
	inline void x(float a) { wxyz.wxyz[1] = a; }

	inline float y() const { return wxyz.wxyz[2]; }
	inline void y(float a) { wxyz.wxyz[2] = a; }

	inline float z() const { return wxyz.wxyz[3]; }
	inline void z(float a) { wxyz.wxyz[3] = a; }

	union {
		float wxyz[4];
//		__m128 sse_data;
	} wxyz;
};

extern const quaternion quaternion_default;
inline void quaternion_init(quaternion* q) { *q = quaternion_default;  }

void toQuaternion(double x, double y, double z, double deg, quaternion* q);
void toQuaternion2(double pitch, double roll, double yaw, quaternion* q);

quaternion quat_mult(const quaternion* q1, const quaternion* q2);
void quaternion_normalize(quaternion* q);
float quat_length(quaternion* q);

quaternion vector3_to_quat(vector3 a);
vector3 vector3_quat_rotate(vector3 v, const quaternion& q);
