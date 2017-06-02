#include <oglShaders.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <memory.h>

static GLuint _currentShaderProgram = 0;


typedef struct shader_source {
	char* vert_file_path;
	char* frag_file_path;
	char* geom_file_path;

	char* vert;
	char* frag;
	char* geom;
} shader_source;

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

static char* read_from_disk(const char* path) {
	FILE* f = fopen(path, "r");
	if (f == NULL) {
		assert(f);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	uint32_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *str = malloc(size + 1);
	uint32_t bytes_read = fread(str, 1, size, f);
	int error = ferror(f);
	fclose(f);

	if (error != 0) {
		printf("File error: %d\n", error);
		free(str);
		str = NULL;
		return str;
	}

	size = bytes_read;
	char* s = realloc(str, size+1);
	assert(s != NULL);
	str = s;
	str[size] = 0;

	return str;
}

static GLuint compile_shader(char* str, GLuint shader_type) {
	GLuint f_shader = glCreateShader(shader_type);
	glShaderSource(f_shader, 1, &str, NULL);
	glCompileShader(f_shader);

	// check for compile errors
	int params = -1;
	glGetShaderiv(f_shader, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(stderr, "ERROR: GL shader index %i did not compile\n", f_shader);
		_print_shader_info_log(f_shader);
		glDeleteShader(f_shader);
		free(str);
		return 0;
	}
	free(str);

	return f_shader;
}

static void setup_shader(HgShader_ogl* s) {
	HgShader_ogl* shader = (HgShader_ogl*)s;
	shader_source* source = shader->program_code;

	GLuint vert_id = 0;
	GLuint frag_id = 0;
	GLuint geom_id = 0;

	if (source->vert) vert_id = compile_shader(source->vert, GL_VERTEX_SHADER);
	if (source->frag) frag_id = compile_shader(source->frag, GL_FRAGMENT_SHADER);
	if (source->geom) geom_id = compile_shader(source->geom, GL_GEOMETRY_SHADER);

	source->vert = source->frag = source->geom = NULL;

	if ((vert_id == 0) && (frag_id == 0) && (geom_id == 0)) return;

	GLuint shader_program = glCreateProgram();
	if (vert_id>0) glAttachShader(shader_program, vert_id);
	if (frag_id>0) glAttachShader(shader_program, frag_id);
	if (geom_id>0) glAttachShader(shader_program, geom_id);
	glLinkProgram(shader_program);

	if (vert_id>0) glDeleteShader(vert_id);
	if (frag_id>0) glDeleteShader(frag_id);
	if (geom_id>0) glDeleteShader(geom_id);

	// check if link was successful
	int params = -1;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(stderr,
			"ERROR: could not link shader programme GL index %u\n",
			shader_program);
		_print_programme_info_log(shader_program);
		return;
	}
	shader->program_id = shader_program;
	shader->ready = 1;
}

static void load_from_disk(HgShader* s) {
	HgShader_ogl* shader = (HgShader_ogl*)s;
	shader_source* source = shader->program_code;

	GLuint vert_id = 0;
	GLuint frag_id = 0;
	GLuint geom_id = 0;

	if (source->vert_file_path) source->vert = read_from_disk(source->vert_file_path);
	if (source->frag_file_path) source->frag = read_from_disk(source->frag_file_path);
	if (source->geom_file_path) source->geom = read_from_disk(source->geom_file_path);

	shader->source_loaded = 1;
}

static void destroy_shader(HgShader* s) {
}

static void enable_shader(HgShader* s) {
	HgShader_ogl* shader = (HgShader_ogl*)s;

	if (shader->source_loaded == 0) return;

	if (shader->program_id==0) setup_shader(shader);
	if (shader->program_id>0) useShaderProgram(shader->program_id);
}

static HgShader_vtable vtable = {
	.load = load_from_disk,
	.destroy = destroy_shader,
	.enable = enable_shader
};

static char* copy_str(char* str) {
	uint32_t size = strlen(str);
	char* s = malloc(size + 1);
	s[size] = 0;
	memcpy(s, str, size);
	return s;
}

void* HGShader_ogl_create(char* vert, char* frag) {
	HgShader_ogl* s = calloc(1, sizeof* s);

	s->_base.vptr = &vtable;

	shader_source* source = calloc(1, sizeof* source);
	source->vert_file_path = copy_str(vert);
	source->frag_file_path = copy_str(frag);
	s->program_code = source;

	return s;
}