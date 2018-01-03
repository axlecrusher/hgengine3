#include <str_utils.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

uint32_t hgstring_append(hgstring* hg, const char* str) {
	uint32_t slen = strlen(str) + 1; //include null
	if ((hg->used + slen) > hg->allocated_len) {
		char * s = realloc(hg->str, hg->allocated_len + 128);
		assert(s != NULL);
		hg->str = s;
		hg->allocated_len += 128;
	}

	memcpy(hg->str + hg->used, str, slen); //includes null
	uint32_t offset = hg->used;
	hg->used += slen;
	return offset;
}

char* str_cat(const char* s1, const char* s2) {
	uint32_t size = strlen(s1) + strlen(s2);
	char* str = malloc(size + 1);
	assert(str != NULL);
	str[0] = 0;
	strcat_s(str, size+1, s1);
	strcat_s(str, size+1, s2);
	return str;
}

char* str_copy(const char* str) {
	uint32_t size = strlen(str);
	char* s = malloc(size + 1);
	s[size] = 0;
	memcpy(s, str, size);
	return s;
}