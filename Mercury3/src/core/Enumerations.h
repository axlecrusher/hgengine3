#pragma once

#include <stdint.h>

enum BUFFER_USE_TYPE : uint8_t {
	BUFFER_STATIC_DRAW = 0,
	BUFFER_DYNAMIC_DRAW,
	BUFFER_STREAM_DRAW
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
