#pragma once

#include <HgInput.h>
#include <MercuryWindow.h>

//make threadsafe?

uint8_t KeyDownMap[512];
HgMouseInput MOUSE_INPUT;

namespace ENGINE {
	namespace INPUT {
		void PumpMessages() {
			MercuryWindow::GetCurrentWindow()->PumpMessages();
		}
	}
}