#pragma once

#include <quaternion.h>
#include <vertex3d.h>

struct SPI
{
	SPI()
		:scale(1.0)
	{}

	quaternion orientation; //16 bytes
	vertex3f position; float scale; //16 bytes
	vertex3f origin; //12 bytes
};
