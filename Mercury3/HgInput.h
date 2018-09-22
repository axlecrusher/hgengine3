#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define KeyDownMap_length 512
extern uint8_t KeyDownMap[];

typedef struct HgMouseInput {
	int16_t dx, dy;
} HgMouseInput;

extern HgMouseInput MOUSE_INPUT;

#ifdef __cplusplus
}

namespace ENGINE {
	namespace INPUT {
		enum KeyCodes
		{
			KEY_UNKNOWN,
			KEY_BACKSPACE,
			KEY_TAB,
			KEY_CLEAR,
			KEY_ENTER,
			KEY_SHIFT,
			KEY_CONTROL,
			KEY_ALT,
			KEY_PAUSE,
			KEY_CAPS,
			KEY_ESCAPE,
			KEY_SPACE,
			KEY_PAGEUP,
			KEY_PAGEDOWN,
			KEY_END,
			KEY_HOME,
			KEY_LEFTARROW,
			KEY_UPARROW,
			KEY_RIGHTARROW,
			KEY_DOWNARROW,
			KEY_SELECT,
			KEY_PRINT,
			KEY_EXE,
			KEY_PRINTSCREEN,
			KEY_INSERT,
			KEY_DELETE,
			KEY_HELP,
			KEY_0,
			KEY_1,
			KEY_2,
			KEY_3,
			KEY_4,
			KEY_5,
			KEY_6,
			KEY_7,
			KEY_8,
			KEY_9,
			KEY_A,
			KEY_B,
			KEY_C,
			KEY_D,
			KEY_E,
			KEY_F,
			KEY_G,
			KEY_H,
			KEY_I,
			KEY_J,
			KEY_K,
			KEY_L,
			KEY_M,
			KEY_N,
			KEY_O,
			KEY_P,
			KEY_Q,
			KEY_R,
			KEY_S,
			KEY_T,
			KEY_U,
			KEY_V,
			KEY_W,
			KEY_X,
			KEY_Y,
			KEY_Z,
			KEY_LWINDOWS,
			KEY_RWINDOWS,
			KEY_NUM0,
			KEY_NUM1,
			KEY_NUM2,
			KEY_NUM3,
			KEY_NUM4,
			KEY_NUM5,
			KEY_NUM6,
			KEY_NUM7,
			KEY_NUM8,
			KEY_NUM9,
			KEY_MULTIPLY,
			KEY_ADD,
			KEY_SUBTRACT,
			KEY_DECIMAL,
			KEY_DIVIDE,
			KEY_F1,
			KEY_F2,
			KEY_F3,
			KEY_F4,
			KEY_F5,
			KEY_F6,
			KEY_F7,
			KEY_F8,
			KEY_F9,
			KEY_F10,
			KEY_F11,
			KEY_F12,
			KEY_F13,
			KEY_F14,
			KEY_F15,
			KEY_F16,
			KEY_F17,
			KEY_F18,
			KEY_F19,
			KEY_F20,
			KEY_F21,
			KEY_F22,
			KEY_F23,
			KEY_F24,
			KEY_NUMLOCK,
			KEY_SCROLLLOCK,
			KEY_LSHIFT,
			KEY_RSHIFT,
			KEY_LCONTROL,
			KEY_RCONTROL,
			KEY_SEMICOLON,
			KEY_PLUS,
			KEY_COMMA,
			KEY_MINUS,
			KEY_PERIOD,
			KEY_QUESTIONMARK,
			KEY_TILDE,
			KEY_L_BRACKET,
			KEY_PIPE,
			KEY_R_BRACKET,
			KEY_QUOTE
		};

		void PumpMessages();
		bool ScanForKeyCombo(KeyCodes key1, KeyCodes key2);
	}
}

#endif