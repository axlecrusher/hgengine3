#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h> 
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

namespace Engine
{
	inline void Log(const char* file, uint32_t line, const char* str, ...)
	{
		char msg[1024];
		char buffer[1024];

		snprintf(msg, 1024, "%s (%d): %s\n", file, line, str);

		va_list args;
		va_start(args, str);
		auto length = vsnprintf(buffer, 1024, msg, args);
		va_end(args);

		fwrite(buffer, sizeof(char) * length, 1, stdout);
	}
}

#define LOG(STR, ...) \
	Log(__FILENAME__, __LINE__, STR, __VA_ARGS__);

