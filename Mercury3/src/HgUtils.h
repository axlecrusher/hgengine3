#pragma once

#define NUM_ARRAY_ELEMENTS(X) (sizeof(X)/sizeof(*X))
#define SIZEOF_ARRAY(X) (*(&X + 1) - X)

#include <stdint.h>

namespace ENGINE {
	void InitEngine();
	void StartWindowSystem(uint16_t width, uint16_t height);
	void EnumberateSymbols();
	void SetRealtimePriority();
}