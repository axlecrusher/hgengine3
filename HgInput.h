#pragma once
#include <stdint.h>

extern uint8_t KeyDownMap[];

typedef struct HgMouseInput {
	int16_t dx, dy;
} HgMouseInput;

extern HgMouseInput MOUSE_INPUT;