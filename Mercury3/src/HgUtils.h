#pragma once

#define NUM_ARRAY_ELEMENTS(X) (sizeof(X)/sizeof(*X))
#define SIZEOF_ARRAY(X) (*(&X + 1) - X)

namespace ENGINE {
	void InitEngine();
	void StartWindowSystem();
	void EnumberateSymbols();
	void SetRealtimePriority();
}