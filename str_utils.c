#include <str_utils.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

char* str_cat(const char* s1, const char* s2) {
	uint32_t size = strlen(s1) + strlen(s2);
	char* str = malloc(size + 1);
	assert(str != NULL);
	str[0] = 0;
	strcat(str, s1);
	strcat(str, s2);
	return str;
}

char* str_copy(const char* str) {
	uint32_t size = strlen(str);
	char* s = malloc(size + 1);
	s[size] = 0;
	memcpy(s, str, size);
	return s;
}