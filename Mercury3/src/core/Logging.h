#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h> 

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

namespace Engine
{
	void Log(FILE* file, const char* srcFile, uint32_t line, const char* str, ...);
}

#define LOG(STR, ...) \
	Engine::Log(stdout, __FILENAME__, __LINE__, STR, __VA_ARGS__);

#define LOG_ERROR(STR, ...) \
	Engine::Log(stderr, __FILENAME__, __LINE__, STR, __VA_ARGS__);
