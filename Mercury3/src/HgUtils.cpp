#include <symbol_enumerator.h>
#include <string.h>

#include <HgUtils.h>

#include <Windows.h>
#include <stdio.h>
#include <HgMessaging.h>

#include <MercuryWindow.h>
#include <RenderBackend.h>

#include <HgInput.h>

static int SymnumCheck(const char * path, const char * name, void * location, long size)
{
	if (strncmp(name, "REGISTER", 8) == 0)
	{
		typedef void(*sf)();
		sf fn = (sf)location;
		fn();
	}
	return 0;
}

namespace ENGINE {
	void InitEngine() {
		EnumberateSymbols();
		SetRealtimePriority();
		InitMessageSystem();

		memset(KeyDownMap, 0, sizeof(uint8_t)*KeyDownMap_length);
	}
	
	void StartWindowSystem() {
		MercuryWindow::MakeWindow();
		Renderer::Init();
	}

	void EnumberateSymbols() {
		EnumerateSymbols(SymnumCheck);
	}

	void SetRealtimePriority() {
		HANDLE h = GetCurrentProcess();
		BOOL b = SetPriorityClass(h, 0x80); //high priority
		if (b == 0) {
			DWORD error = GetLastError();
			fprintf(stderr, "Could not set process priority class. (%d)\n", error);
		}

	}
}