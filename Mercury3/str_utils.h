#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct hgstring {
	char* str;
	uint32_t allocated_len;
	uint32_t used;
} hgstring;

uint32_t hgstring_append(hgstring* hg, const char* str);


char* str_cat(const char* s1, const char* s2);
char* str_copy(const char* str);

#ifdef __cplusplus
}
#endif