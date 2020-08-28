#pragma once

#include <stdint.h>

struct GpuBufferId
{
	GpuBufferId() : value(0)
	{}

	~GpuBufferId() {
		Destroy();
	}

	GpuBufferId(const GpuBufferId& rhs) = delete;
	GpuBufferId(GpuBufferId&& rhs)
	{
		Destroy();
		value = rhs.value;
		rhs.value = 0;
	}

	const GpuBufferId& operator=(const GpuBufferId& rhs) = delete;
	const GpuBufferId& operator=(GpuBufferId&& rhs)
	{
		Destroy();
		value = rhs.value;
		rhs.value = 0;
		return *this;
	}

	inline bool operator==(uint32_t x) const { return value == x; }

	bool Init();
	void Destroy();

	uint32_t value;
};