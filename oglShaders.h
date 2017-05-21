#pragma once

#include <glew.h>
#include <stdint.h>

GLuint shaders_load(const char* path, uint32_t shader_type);
void useShaderProgram(GLuint id);
void _print_programme_info_log(GLuint programme);
void _print_shader_info_log(GLuint idx);