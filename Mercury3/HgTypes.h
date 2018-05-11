#pragma once

#include <vertex.h>
#include <vertex3d.h>

typedef vertex3d point;
typedef vertex3d vector3;

#define L_VERTEX	0
#define L_NORMAL	1
#define L_UV		2
#define L_COLOR		3
#define L_TANGENT	4

#ifdef __cplusplus
extern "C" {
#endif

//	typedef vertex point;
//	typedef vertex vector3;

	typedef struct color {
		uint8_t r, g, b, a;
	} color;

	typedef struct uv_coord {
		uint16_t u, v;
	} uv_coord;

	typedef struct normal {
		float x, y, z;
	} normal;

	typedef struct tangent {
		float x, y, z, w;
	} tangent;
#ifdef __cplusplus
}
#endif