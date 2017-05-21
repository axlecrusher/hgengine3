#include <oglShaders.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

static GLuint _currentShaderProgram = 0;

void _print_shader_info_log(GLuint idx) {
	int max_length = 2048;
	int actual_length = 0;
	char shader_log[2048];
	glGetShaderInfoLog(idx, max_length, &actual_length, shader_log);
	printf("shader info log for GL index %u:\n%s\n", idx, shader_log);
}

void _print_programme_info_log(GLuint programme) {
	int max_length = 2048;
	int actual_length = 0;
	char program_log[2048];
	glGetProgramInfoLog(programme, max_length, &actual_length, program_log);
	printf("program info log for GL index %u:\n%s", programme, program_log);
}

void useShaderProgram(GLuint id) {
	//WARNING: switching shaders takes a BOATLOAD of time. it is best to switch as little as possible.
	//may even want to render all elements with the same shader before moving onto next element...
	if (_currentShaderProgram == id) return;
	_currentShaderProgram = id;
	glUseProgram(id);
//	printf("shader %d\n", id);
}


GLuint shaders_load(const char* path, uint32_t shader_type) {
	FILE* f = fopen(path, "r");
	if (f == NULL) {
		assert(f);
		return 0;
	}

	fseek(f, 0, SEEK_END);
	GLint size = ftell(f);
	fseek(f, 0, SEEK_SET);

	GLchar *str = malloc(size + 1);
	fread(str, size, 1, f);
	fclose(f);
	str[size] = 0;

//	printf("%s\n", str);

	GLuint f_shader = glCreateShader(shader_type);
	glShaderSource(f_shader, 1, &str, &size);
	free(str);
	glCompileShader(f_shader);

	// check for compile errors
	int params = -1;
	glGetShaderiv(f_shader, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(stderr, "ERROR: GL shader index %i did not compile\n", f_shader);
		_print_shader_info_log(f_shader);
		glDeleteShader(f_shader);
		return 0;
	}

	return f_shader;
}