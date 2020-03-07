#pragma once

#define NUM_ARRAY_ELEMENTS(X) (sizeof(X)/sizeof(*X))
#define SIZEOF_ARRAY(X) (*(&X + 1) - X)

#include <stdint.h>
#include <MercuryWindow.h>

namespace ENGINE {
	void InitEngine();
	void StartWindowSystem(MercuryWindow::Dimensions d);
	void EnumberateSymbols();
	void SetRealtimePriority();
}