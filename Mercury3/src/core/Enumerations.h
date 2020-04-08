#pragma once

#include <stdint.h>

enum BUFFER_USE_TYPE : uint8_t {
	BUFFER_DRAW_STATIC = 0,
	BUFFER_DRAW_DYNAMIC,
	BUFFER_DRAW_STREAM
};

enum BUFFER_TYPE : uint8_t
{
	VERTEX_ATTRIBUTES,
	TEXTURE_BUFFER
};
namespace HgEngine
{

	enum PrimitiveType : uint8_t
	{
		TRIANGLES,
		LINES,
		POINTS
	};

}
