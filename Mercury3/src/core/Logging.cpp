#pragma once

#include <Logging.h>

namespace Engine
{
	void Log(FILE* file, const char* srcFile, uint32_t line, const char* str, ...)
	{
		char msg[1024];
		char buffer[1024];

		snprintf(msg, 1024, "%s (%d): %s\n", srcFile, line, str);

		va_list args;
		va_start(args, str);
		auto length = vsnprintf(buffer, 1024, msg, args);
		va_end(args);

		fwrite(buffer, sizeof(char) * length, 1, stdout);
	}
}
