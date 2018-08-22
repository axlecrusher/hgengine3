#include <oglShaders.h>
#include <oglDisplay.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <memory.h>
#include <str_utils.h>
#include <string.h>

#include <OGL/OGLGpuBuffer.h>

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
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *str = (char*)malloc(size + 1);
	size_t bytes_read = fread(str, 1, size, f);
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

static GLuint compile_shader(const char* str, GLuint shader_type) {
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
		return 0;
	}

	return f_shader;
}

void HgOglShader::setup_shader(HgOglShader* shader) {
	shader_source* source = shader->m_shaderSource.get();

	GLuint vert_id = 0;
	GLuint frag_id = 0;
	GLuint geom_id = 0;

	if (!source->vert_source.empty()) vert_id = compile_shader(source->vert_source.c_str(), GL_VERTEX_SHADER);
	if (!source->frag_source.empty()) frag_id = compile_shader(source->frag_source.c_str(), GL_FRAGMENT_SHADER);
	if (!source->geom_source.empty()) geom_id = compile_shader(source->geom_source.c_str(), GL_GEOMETRY_SHADER);

	source->vert_source = source->frag_source = source->geom_source = "";

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
	GLint params = -1;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf(stderr,
			"ERROR: could not link shader programme GL index %u\n",
			shader_program);
		_print_programme_info_log(shader_program);
		return;
	}
	shader->program_id = shader_program;
	shader->m_loadState = LoadState::READY;


	GLint size; // size of the variable
	GLenum type; // type of the variable (float, vec3 or mat4, etc)

	GLchar name[64]; // variable name in GLSL
	GLsizei length; // name length

	//create list of uniforms
	GLint uniform_count = 0;
	glGetProgramiv(shader_program, GL_ACTIVE_UNIFORMS, &uniform_count);

	memset(shader->m_uniformLocations, -1, sizeof(*shader->m_uniformLocations)*U_UNIFORM_COUNT);

	for (GLuint i = 0; i < uniform_count; i++)
	{
		glGetActiveUniform(shader_program, i, 64, &length, &size, &type, name);
//		printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
		for (int j = 0; j < U_UNIFORM_COUNT; j++) {
			if (strcmp(name, UniformString[j]) == 0) {
				shader->m_uniformLocations[j] = glGetUniformLocation(shader_program, name);
				break;
			}
			if (j == (U_UNIFORM_COUNT - 1)) fprintf(stderr, "Unknown uniform %s", name);
		}
	}
}

std::unique_ptr<HgShader> HgOglShader::Create(const char* vert, const char* frag) {
	std::unique_ptr<HgOglShader> s = std::make_unique<HgOglShader>();


	auto source = std::make_unique<shader_source>();
	source->vert_file_path = str_copy(vert);
	source->frag_file_path = str_copy(frag);
	s->setProgramCode(source);

	return s;
}

HgOglShader::HgOglShader()
	:program_id(0), m_loadState(LoadState::NOT_LOADED)
{
	memset(m_uniformLocations, -1, sizeof(m_uniformLocations));
}

HgOglShader::~HgOglShader() {
	destroy();
}

void HgOglShader::load() {
	if (!m_shaderSource->vert_file_path.empty()) m_shaderSource->vert_source = read_from_disk(m_shaderSource->vert_file_path.c_str());
	if (!m_shaderSource->frag_file_path.empty()) m_shaderSource->frag_source = read_from_disk(m_shaderSource->frag_file_path.c_str());
	if (!m_shaderSource->geom_file_path.empty()) m_shaderSource->geom_source = read_from_disk(m_shaderSource->geom_file_path.c_str());

	m_loadState = LoadState::SOURCE_LOADED;
}

void HgOglShader::destroy() {
	if (program_id > 0) glDeleteProgram(program_id);
	program_id = 0;
}

void HgOglShader::enable() {
	if (m_loadState == LoadState::NOT_LOADED) return;

	if (program_id == 0 || m_loadState == LoadState::SOURCE_LOADED) setup_shader(this);
	if (program_id>0) useShaderProgram(program_id);
}

void HgOglShader::setGlobalUniforms(const HgCamera& c) {
	const GLuint old_program = _currentShaderProgram;

	if (old_program == program_id) {
		//if we are running a shader that is already enabled, just set the values
		sendGlobalUniformsToGPU(c);
		return;
	}

	//Log warning about being slow and change to this program
	fprintf(stderr, "Warning (%s): Temporary shader context change.\n", __FUNCTION__);
	enable();

	sendGlobalUniformsToGPU(c);

	useShaderProgram(old_program); //change back to previous program
}

void HgOglShader::sendGlobalUniformsToGPU(const HgCamera& c) {
	//can I send as single global transform matrix combining all transforms?
	if (m_uniformLocations[U_PROJECTION] > -1) glUniformMatrix4fv(m_uniformLocations[U_PROJECTION], 1, GL_FALSE, _projection);
	if (m_uniformLocations[U_CAMERA_ROT] > -1) glUniform4f(m_uniformLocations[U_CAMERA_ROT], c.rotation.x(), c.rotation.y(), c.rotation.z(), c.rotation.w());
	if (m_uniformLocations[U_CAMERA_POS] > -1) glUniform3f(m_uniformLocations[U_CAMERA_POS], c.position.x(), c.position.y(), c.position.z());
}

void HgOglShader::setLocalUniforms(const quaternion* rotation, const point* position, float scale, const point* origin, const RenderData* rd) {
	GLuint old_program = _currentShaderProgram;

	if (old_program == program_id) {
		sendLocalUniformsToGPU(rotation, position, scale, origin, rd);
		return;
	}

	//Log warning about being slow and change to this program
	fprintf(stderr, "Warning (%s): Temporary shader context change.\n", __FUNCTION__);
	enable();

	sendLocalUniformsToGPU(rotation, position, scale, origin, rd);

	useShaderProgram(old_program); //change back to previous program
}

void HgOglShader::sendLocalUniformsToGPU(const quaternion* rotation, const point* position, float scale, const point* origin, const RenderData* rd) {
	OGLRenderData* oglrd = (OGLRenderData*)rd;

	//this could be replaced with a single matrix uniform for setting the world position and rotation of the object. not check would be needed as it is required information for rendering
	if (m_uniformLocations[U_ROTATION] > -1) glUniform4f(m_uniformLocations[U_ROTATION], rotation->x(), rotation->y(), rotation->z(), rotation->w());
	if (m_uniformLocations[U_POSITION] > -1) glUniform4f(m_uniformLocations[U_POSITION], position->x(), position->y(), position->z(), scale);
	if (m_uniformLocations[U_ORIGIN] > -1) glUniform3f(m_uniformLocations[U_ORIGIN], origin->x(), origin->y(), origin->z());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::DIFFUSE]);
	if ((m_uniformLocations[U_DIFFUSE_TEXTURE] > -1) && (oglrd->textureID[HgTexture::DIFFUSE] > 0)) {
		//		glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::DIFFUSE]);
		glUniform1i(m_uniformLocations[U_DIFFUSE_TEXTURE], 0);
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::SPECULAR]);
	if ((m_uniformLocations[U_SPECULAR_TEXTURE] > -1) && (oglrd->textureID[HgTexture::SPECULAR] > 0)) {
		//		glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::SPECULAR]);
		glUniform1i(m_uniformLocations[U_SPECULAR_TEXTURE], 1);
	}

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::NORMAL]);
	if ((m_uniformLocations[U_NORMAL_TEXTURE] > -1) && (oglrd->textureID[HgTexture::NORMAL] > 0)) {
		//		glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::SPECULAR]);
		glUniform1i(m_uniformLocations[U_NORMAL_TEXTURE], 2);
	}

	glActiveTexture(GL_TEXTURE3);
	if ((m_uniformLocations[U_BUFFER_OBJECT1] > -1) && (oglrd->gpuBuffer != nullptr)) {
		OGLHgGPUBuffer* api = (OGLHgGPUBuffer*)oglrd->gpuBuffer->apiImpl();
		if (oglrd->gpuBuffer->NeedsUpdate()) {
			//api->OGLHgGPUBuffer::SendToGPU(oglrd->gpuBuffer.get()); //no vtable lookup
			api->OGLHgGPUBuffer::SendToGPU(oglrd->gpuBuffer); //no vtable lookup
		}
		api->OGLHgGPUBuffer::Bind(); //no vtable lookup
		glUniform1i(m_uniformLocations[U_BUFFER_OBJECT1], 3);
	}
}