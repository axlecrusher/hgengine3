#include <symbol_enumerator.h>
#include <string.h>

#include <Windows.h>
#include <stdio.h>

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