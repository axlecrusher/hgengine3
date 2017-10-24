#include <oglShaders.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <memory.h>
#include <str_utils.h>
#include <string.h>

static GLuint _currentShaderProgram = 0;

#pragma warning(disable:4996)

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

	char *str = (char*)malloc(size + 1);
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
	char* realloc_ptr = (char*)realloc(str, size+1);
	assert(realloc_ptr != NULL);
	str = realloc_ptr;
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

void HgOglShader::setup_shader(HgOglShader* shader) {
	shader_source* source = shader->program_code;

	GLuint vert_id = 0;
	GLuint frag_id = 0;
	GLuint geom_id = 0;

	if (source->vert_source) vert_id = compile_shader(source->vert_source, GL_VERTEX_SHADER);
	if (source->frag_source) frag_id = compile_shader(source->frag_source, GL_FRAGMENT_SHADER);
	if (source->geom_source) geom_id = compile_shader(source->geom_source, GL_GEOMETRY_SHADER);

	source->vert_source = source->frag_source = source->geom_source = NULL;

	if ((vert_id == 0) && (frag_id == 0) && (geom_id == 0)) return;


	GLuint shader_program = shader->program_id;
	if (shader_program==0) shader_program = glCreateProgram();

	if (vert_id>0) glAttachShader(shader_program, vert_id);
	if (frag_id>0) glAttachShader(shader_program, frag_id);
	if (geom_id>0) glAttachShader(shader_program, geom_id);
	glLinkProgram(shader_program);

	if (vert_id > 0) {
		glDetachShader(shader_program, vert_id);
		glDeleteShader(vert_id);
	}
	if (frag_id > 0) {
		glDetachShader(shader_program, frag_id);
		glDeleteShader(frag_id);
	}
	if (geom_id > 0) {
		glDetachShader(shader_program, geom_id);
		glDeleteShader(geom_id);
	}

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
	shader->source_loaded = 2;


	GLint size; // size of the variable
	GLenum type; // type of the variable (float, vec3 or mat4, etc)

	GLchar name[64]; // variable name in GLSL
	GLsizei length; // name length

	//create list of uniforms
	GLint uniform_count = 0;
	glGetProgramiv(shader_program, GL_ACTIVE_UNIFORMS, &uniform_count);

	memset(shader->uniform_locations, -1, sizeof(*shader->uniform_locations)*U_UNIFORM_COUNT);

	for (int32_t i = 0; i < uniform_count; i++)
	{
		glGetActiveUniform(shader_program, (GLuint)i, 64, &length, &size, &type, name);
//		printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
		for (uint8_t j = 0; j < U_UNIFORM_COUNT; j++) {
			if (strcmp(name, UniformString[j]) == 0) {
				shader->uniform_locations[j] = glGetUniformLocation(shader_program, name);
				break;
			}
			if (j == (U_UNIFORM_COUNT - 1)) fprintf(stderr, "Unknown uniform %s", name);
		}
	}
}

std::unique_ptr<HgShader> HgOglShader::Create(const char* vert, const char* frag) {
	std::unique_ptr<HgOglShader> s = std::make_unique<HgOglShader>();

	shader_source* source = new shader_source();
	source->vert_file_path = str_copy(vert);
	source->frag_file_path = str_copy(frag);
	s->setProgramCode(source);

	return s;
}

HgOglShader::HgOglShader()
	:program_id(0), source_loaded(false), program_code(nullptr)
{
	memset(uniform_locations, 0, sizeof(uniform_locations));
}

HgOglShader::~HgOglShader() {
	destroy();
}

void HgOglShader::load() {
	if (program_code->vert_file_path) program_code->vert_source = read_from_disk(program_code->vert_file_path);
	if (program_code->frag_file_path) program_code->frag_source = read_from_disk(program_code->frag_file_path);
	if (program_code->geom_file_path) program_code->geom_source = read_from_disk(program_code->geom_file_path);

	source_loaded = 1;
}

void HgOglShader::destroy() {
	if (program_id > 0) glDeleteProgram(program_id);
	program_id = 0;
}

void HgOglShader::enable() {
	if (source_loaded == 0) return;

	if (program_id == 0 || source_loaded == 1) setup_shader(this);
	if (program_id>0) useShaderProgram(program_id);
}

void HgOglShader::setGlobalUniforms(const HgCamera& c) {
	GLuint old_program = _currentShaderProgram;
	if (old_program != program_id) {
		//Log warning about being slow and change to this program
		fprintf(stderr, "Warning (%s): Temporary shader context change.\n", __FUNCTION__);
		enable();
	}

	if (uniform_locations[U_PROJECTION] > -1) glUniformMatrix4fv(uniform_locations[U_PROJECTION], 1, GL_TRUE, _projection);
	if (uniform_locations[U_CAMERA_ROT] > -1) glUniform4f(uniform_locations[U_CAMERA_ROT], c.rotation.x(), c.rotation.y(), c.rotation.z(), c.rotation.w());
	if (uniform_locations[U_CAMERA_POS] > -1) glUniform3f(uniform_locations[U_CAMERA_POS], c.position.components.x, c.position.components.y, c.position.components.z);

	if (old_program != program_id) {
		useShaderProgram(old_program); //change back to previous program
	}
}

void HgOglShader::setLocalUniforms(const quaternion* rotation, const point* position, float scale, const point* origin, const RenderData* rd) {
	OGLRenderData* oglrd = (OGLRenderData*)rd;
	GLuint old_program = _currentShaderProgram;
	if (old_program != program_id) {
		//Log warning about being slow and change to this program
		fprintf(stderr, "Warning (%s): Temporary shader context change.\n", __FUNCTION__);
		enable();
	}

	if (uniform_locations[U_ROTATION] > -1) glUniform4f(uniform_locations[U_ROTATION], rotation->x(), rotation->y(), rotation->z(), rotation->w());
	if (uniform_locations[U_POSITION] > -1) glUniform4f(uniform_locations[U_POSITION], position->components.x, position->components.y, position->components.z, scale);
	if (uniform_locations[U_ORIGIN] > -1) glUniform3f(uniform_locations[U_ORIGIN], origin->components.x, origin->components.y, origin->components.z);

	if ((uniform_locations[U_DIFFUSE_TEXTURE] > -1) && (oglrd->textureID[HgTexture::DIFFUSE] > 0)) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::DIFFUSE]);
		glUniform1i(uniform_locations[U_DIFFUSE_TEXTURE], 0);
	}

	if ((uniform_locations[U_SPECULAR_TEXTURE] > -1) && (oglrd->textureID[HgTexture::SPECULAR] > 0)) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::SPECULAR]);
		glUniform1i(uniform_locations[U_SPECULAR_TEXTURE], 1);
	}

	if ((uniform_locations[U_NORMAL_TEXTURE] > -1) && (oglrd->textureID[HgTexture::NORMAL] > 0)) {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::NORMAL]);
		glUniform1i(uniform_locations[U_NORMAL_TEXTURE], 2);
	}

	if (old_program != program_id) {
		useShaderProgram(old_program); //change back to previous program
	}
}