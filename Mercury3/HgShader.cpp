#include <HgShader.h>
#include <HgElement.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <FileWatch.h>
#include <str_utils.h>
#include <stdio.h>

#pragma warning(disable:4996)

#define MAX_SHADERS 1000

HgShader::createShaderCallback HgShader::Create = nullptr;

typedef struct shader_entry {
	uint32_t use_count;
	HgShader* shader;

	char* vertex_path;
	char* frag_path;
} shader_entry;

/* keep shader_names seperate, we iterate through the list, want it cached*/
static char* shader_names[MAX_SHADERS] = { NULL }; //replace strings with CRC32?
shader_entry shader_list[MAX_SHADERS];

static void ShaderFileChanged(void* data) {
	shader_entry* entry = (shader_entry*)data;
	fprintf(stderr, "Shader file changed:%s\n", entry->frag_path);
	entry->shader->load();
}

HgShader* HgShader::acquire(const char* vert, const char* frag) {
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
	shader_list[i].shader = HgShader::Create(vert,frag);
	shader_list[i].frag_path = str_cat(frag, "");
	shader_list[i].vertex_path = str_cat(vert, "");
	WatchFileForChange(frag, ShaderFileChanged, shader_list+i);
	WatchFileForChange(vert, ShaderFileChanged, shader_list + i);

	shader_list[i].shader->load();

	return shader_list[i].shader;
}

void HgShader::release(HgShader* s) {
	uint32_t i = 0;
	for (i = 0; i < MAX_SHADERS; ++i) {
		if (s == shader_list[i].shader) {
			shader_list[i].use_count--;
			return;
		}
	}
}