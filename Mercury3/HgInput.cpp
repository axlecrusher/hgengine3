#pragma once

#include <HgInput.h>
#include <MercuryWindow.h>



//make threadsafe?


bool KeyDownMap[KeyDownMap_length];
bool KeyRepeatMap[KeyDownMap_length];

HgMouseInput MOUSE_INPUT;

namespace ENGINE {
	namespace INPUT {
		void PumpMessages() {
			auto window = MercuryWindow::GetCurrentWindow();
			window->PumpMessages();
			window->ScanForKeystrokes();
		}

		bool ScanForKeyCombo(KeyCodes key1, KeyCodes key2) {
			bool r = KeyDownMap[key1] && KeyDownMap[key2];
			if (!r) return false;

			bool others = false;

			for (int i = 0; i < KeyDownMap_length; ++i) {
				bool b = (i == key1) || (i == key2);
				others |= (!b && KeyDownMap[i]>0);
			}

			return !others;
		}

		bool isKeyDown(KeyCodes key, bool ignoreRepeat) {
			return KeyDownMap[key] && !(KeyRepeatMap[key] && ignoreRepeat);
		}
	}
}