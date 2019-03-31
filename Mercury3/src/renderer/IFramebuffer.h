#pragma once

#include <stdint.h>

class IFramebuffer
{
public:
	virtual ~IFramebuffer()
	{}

	virtual bool Init(uint16_t width, uint16_t height) = 0;
	virtual void Enable() = 0;
	virtual void Disable() = 0;
};