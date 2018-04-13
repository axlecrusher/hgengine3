#pragma once

#include <vertex.h>
#include <vertex3d.h>

typedef vertex3d point;
typedef vertex3d vector3;

#ifdef __cplusplus
extern "C" {
#endif

//	typedef vertex point;
//	typedef vertex vector3;

	typedef struct viewport {
		uint16_t x, y, width, height;
	} viewport;

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