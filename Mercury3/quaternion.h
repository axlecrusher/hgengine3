#pragma once

#include <math.h>

typedef struct quaternion {
	float w;
	float x;
	float y;
	float z;
} quaternion;

inline void quaternion_init(quaternion* q) { q->x = q->y = q->z = 0; q->w = 1.0f; }

void toQuaternion(double x, double y, double z, double deg, quaternion* q);
void toQuaternion2(double pitch, double roll, double yaw, quaternion* q);

quaternion quat_mult(const quaternion* q1, const quaternion* q2);
void quaternion_normalize(quaternion* q);
float quat_length(quaternion* q);