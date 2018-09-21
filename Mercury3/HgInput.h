#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern uint8_t KeyDownMap[];

typedef struct HgMouseInput {
	int16_t dx, dy;
} HgMouseInput;

extern HgMouseInput MOUSE_INPUT;

#ifdef __cplusplus
}

namespace ENGINE {
	namespace INPUT {
		void PumpMessages();
	}
}

#endif