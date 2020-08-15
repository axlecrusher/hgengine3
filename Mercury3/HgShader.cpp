#include <HgShader.h>
#include <HgEntity.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <FileWatch.h>
#include <str_utils.h>
#include <stdio.h>
#include <Logging.h>

#include <../oglShaders.h>

#pragma warning(disable:4996)

#define MAX_SHADERS 1000

HgShader::createShaderCallback HgShader::Create = nullptr;

typedef struct shader_entry {
	uint32_t use_count;
	std::unique_ptr<HgShader> shader;
	std::string vertex_path, frag_path;
} shader_entry;

/* keep shader_names seperate, we iterate through the list, want it cached*/
static char* shader_names[MAX_SHADERS] = { NULL }; //replace strings with CRC32?
shader_entry shader_list[MAX_SHADERS];

static void ShaderFileChanged(void* data) {
	shader_entry* entry = (shader_entry*)data;
	LOG_ERROR("Shader file changed:%s", entry->frag_path.c_str());
	entry->shader->load();
}

HgShader* HgShader::acquire(HgShader* shader)
{
	for (int i = 0; i < MAX_SHADERS; ++i)
	{
		if (shader_list[i].shader.get() == shader)
		{
			if (shader_list[i].use_count > 0)
			{
				shader_list[i].use_count++;
				return shader;
			}
			return nullptr;
		}
	}
	return nullptr;
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
			return shader_list[i].shader.get();
		}
	}

	assert(funused != 0xFFFFFFFF); //out of shader spaces! increase MAX_SHADERS

	i = funused;
	shader_names[i] = name;
	shader_list[i].use_count = 1;
	shader_list[i].shader = HgShader::Create(vert, frag);
	shader_list[i].frag_path = std::string(frag);
	shader_list[i].vertex_path = std::string(vert);

	void* addr1 = shader_list + i;
	WatchFileForChange(frag, [addr1]() {
		ShaderFileChanged(addr1);
	});

	WatchFileForChange(vert, [addr1]() {
		ShaderFileChanged(addr1);
	});

	shader_list[i].shader->load();

	return shader_list[i].shader.get();
}

void HgShader::release(HgShader* s) {
	uint32_t i = 0;
	for (i = 0; i < MAX_SHADERS; ++i) {
		if (s == shader_list[i].shader.get()) {
			shader_list[i].use_count--;
			return;
		}
	}
}

int32_t HgShader::getAttributeLocation(const std::string& name) const 
{
	auto itr = m_attribLocations.find(name);
	if (itr == m_attribLocations.end())
	{
		//get the attribute location from the shader
		OGLShaderHandle program_id = getProgramHandle();
		const auto attribLocation = glGetAttribLocation(program_id, name.c_str());
		m_attribLocations[name] = attribLocation;
		return attribLocation;
	}

	return itr->second;

}