#include <symbol_enumerator.h>
#include <string.h>

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
}