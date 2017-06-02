#pragma once

#include <glew.h>
#include <stdint.h>

#include <HgShader.h>

typedef struct HgShader_ogl {
	HgShader _base;
	GLuint program_id;
	uint8_t source_loaded;

	//other things not needed often
	void* program_code;
} HgShader_ogl;


//GLuint shaders_load(const char* path, uint32_t shader_type);
//void useShaderProgram(GLuint id);
//void _print_programme_info_log(GLuint programme);
//void _print_shader_info_log(GLuint idx);

HgShader* HGShader_ogl_create(const char* vert, const char* frag);