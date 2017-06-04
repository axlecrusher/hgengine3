#include <HgShader.h>
#include <HgElement.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_SHADERS 1000

HgShader*(*_create_shader)(const char* vert, const char* frag);

typedef struct shader_entry {
	uint32_t use_count;
	HgShader* shader;
} shader_entry;

/* keep shader_names seperate, we iterate through the list, want it cached*/
static char* shader_names[MAX_SHADERS] = { NULL }; //replace strings with CRC32?
shader_entry shader_list[MAX_SHADERS];

static char* str_cat(const char* s1, const char* s2) {
	uint32_t size = strlen(s1) + strlen(s2);
	char* str = malloc(size + 1);
	assert(str != NULL);
	str[0] = 0;
	strcat(str, s1);
	strcat(str, s2);
	return str;
}

HgShader* HGShader_acquire(char* vert, char* frag) {
	uint32_t i = 0;
	char* name = str_cat(vert, frag);
	uint32_t funused = 0xFFFFFFFF;
	for (i = 0; i < MAX_SHADERS; ++i) {
		if (shader_names[i] == NULL && funused == 0xFFFFFFFF) funused = i;
		if (shader_names[i] == NULL) continue;
		if (strcmp(name, shader_names[i]) == 0) {
			free(name);
			shader_list[i].use_count++;
			return shader_list[i].shader;
		}
	}

	assert(funused != 0xFFFFFFFF); //out of shader spaces! increase MAX_SHADERS

	i = funused;
	shader_names[i] = name;
	shader_list[i].use_count = 1;
	shader_list[i].shader = _create_shader(vert,frag);

	VCALL(shader_list[i].shader, load);

	return shader_list[i].shader;
}

void HGShader_release(HgShader* s) {
	uint32_t i = 0;
	for (i = 0; i < MAX_SHADERS; ++i) {
		if (s == shader_list[i].shader) {
			shader_list[i].use_count--;
			return;
		}
	}
}