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
#include <RenderBackend.h>

#include <OGLBackend.h>

static GLuint _currentShaderProgram = 0;

std::unordered_map <std::string, UniformLocations > UniformStringMap = {
	{"rotation", UniformLocations::U_ROTATION },
	{"translation", UniformLocations::U_POSITION },
	{"view", UniformLocations::U_VIEW },
	{"projection", UniformLocations::U_PROJECTION },
	{"cam_rot", UniformLocations::U_CAMERA_ROT },
	{"cam_position", UniformLocations::U_CAMERA_POS },
	{"origin", UniformLocations::U_ORIGIN },
	{"diffuseTex", UniformLocations::U_DIFFUSE_TEXTURE },
	{"specularTex", UniformLocations::U_SPECULAR_TEXTURE },
	{"normalTex", UniformLocations::U_NORMAL_TEXTURE },
	{"bufferObject1", UniformLocations::U_BUFFER_OBJECT1 },
	{"modelMatrix", UniformLocations::U_MODEL_MATRIX },
	{"matrices[0]", UniformLocations::U_MATRICES },
	{"remainingTime", UniformLocations::U_TIME_REMAIN }
};

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

static std::string read_from_disk(const char* path) {
	std::string source;
	FILE* f = fopen(path, "r");
	if (f == NULL) {
		return source;
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

	source = std::string(str, bytes_read);
	free(str);

	return source;
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


	OGLShaderHandle shader_program = shader->getProgramHandle();
	if (shader_program == 0) shader_program = glCreateProgram();

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

	shader->m_handle = shader_program;
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
		const auto itr = UniformStringMap.find(std::string(name, length));

		if (itr != UniformStringMap.end())
		{
			const auto idx = itr->second;
			shader->m_uniformLocations[idx] = glGetUniformLocation(shader_program, name);
		}
		else
		{
			fprintf(stderr, "HgShaders: Unknown uniform \"%s\"\n", name);
		}
	}
}

std::unique_ptr<HgShader> HgOglShader::Create(const char* vert, const char* frag) {
	std::unique_ptr<HgOglShader> s = std::make_unique<HgOglShader>();


	auto source = std::make_unique<shader_source>();

	source->vert_file_path = vert;
	source->frag_file_path = frag;

	std::hash<std::string> hash;
	s->setUniqueId(hash(source->vert_file_path + source->frag_file_path));

	s->setProgramCode(source);

	return s;
}

HgOglShader::HgOglShader()
	:HgShader(), m_loadState(LoadState::NOT_LOADED)
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
	OGLShaderHandle program_id = getProgramHandle();
	if (program_id > 0) glDeleteProgram(program_id);
	m_handle = 0;
}

bool HgOglShader::compile() {
	if (m_loadState == LoadState::NOT_LOADED)
	{
		fprintf(stderr, "Can not compile, source not loaded\n");
		return false;
	}

	OGLShaderHandle program_id = getProgramHandle();

	if (program_id == 0 || m_loadState == LoadState::SOURCE_LOADED)
	{
		setup_shader(this);
		return true;
	}
	return false;
}

void HgOglShader::enable() {
	OGLShaderHandle program_id = getProgramHandle();
	if (program_id == 0 || m_loadState == LoadState::SOURCE_LOADED)
	{
		compile();
		program_id = getProgramHandle();
	}

	if (program_id>0) useShaderProgram(program_id);
}

void HgOglShader::setLocalUniforms(const ShaderUniforms& uniforms) {
	GLuint old_program = _currentShaderProgram;

	OGLShaderHandle program_id = getProgramHandle();
		if (old_program == program_id) {
		sendLocalUniformsToGPU(uniforms);
		return;
	}

	//Log warning about being slow and change to this program
	fprintf(stderr, "Warning (%s): setLocalUniforms forcing temporary shader context change.\n", __FUNCTION__);
	enable();

	sendLocalUniformsToGPU(uniforms);

	useShaderProgram(old_program); //change back to previous program
}

void HgOglShader::uploadMatrices(const float* worldSpaceMatrix, const HgMath::mat4f& projection, const HgMath::mat4f& view) {
	using namespace HgMath;
	constexpr const int matrixCount = 3;

	float mm[16 * matrixCount];

	if (m_uniformLocations[U_MATRICES] <= -1) return; //sometimes opengl will optimize the matrices uniform out

	//rd->getWorldSpaceMatrix(mm);
	memcpy(mm, worldSpaceMatrix, sizeof(float) * 16);
	//modelView.store(mm);
	projection.store(mm + 16);
	view.store(mm + 32);
	glUniformMatrix4fv(m_uniformLocations[U_MATRICES], matrixCount, GL_FALSE, mm);
}

void HgOglShader::sendLocalUniformsToGPU(const ShaderUniforms& uniforms) {
	auto renderer = (OGLBackend*)RENDERER();

	if (uniforms.material)
	{
		auto textId = uniforms.material->getGPUTextureHandle(HgTexture::DIFFUSE);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, textId);
		if ((m_uniformLocations[U_DIFFUSE_TEXTURE] > -1) && (textId > 0)) {
			renderer->BindTexture(0, textId, GL_TEXTURE_2D);
			//		glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::DIFFUSE]);
			glUniform1i(m_uniformLocations[U_DIFFUSE_TEXTURE], 0);
		}

		textId = uniforms.material->getGPUTextureHandle(HgTexture::SPECULAR);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, textId);
		if ((m_uniformLocations[U_SPECULAR_TEXTURE] > -1) && (textId > 0)) {
			renderer->BindTexture(1, textId, GL_TEXTURE_2D);
			//		glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::SPECULAR]);
			glUniform1i(m_uniformLocations[U_SPECULAR_TEXTURE], 1);
		}

		textId = uniforms.material->getGPUTextureHandle(HgTexture::NORMAL);
		//glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, textId);
		if ((m_uniformLocations[U_NORMAL_TEXTURE] > -1) && (textId > 0)) {
			renderer->BindTexture(2, textId, GL_TEXTURE_2D);
			//		glBindTexture(GL_TEXTURE_2D, oglrd->textureID[HgTexture::SPECULAR]);
			glUniform1i(m_uniformLocations[U_NORMAL_TEXTURE], 2);
		}
	}

	//auto gpuBuffer = uniforms.gpuBuffer;
	//if ((m_uniformLocations[U_BUFFER_OBJECT1] > -1) && (gpuBuffer != nullptr)) {
	//	OGLHgGPUBuffer* api = (OGLHgGPUBuffer*)gpuBuffer->apiImpl();
	//	if (gpuBuffer->NeedsLoadToGPU()) {
	//		//api->OGLHgGPUBuffer::SendToGPU(oglrd->gpuBuffer.get()); //no vtable lookup
	//		api->OGLHgGPUBuffer::SendToGPU(gpuBuffer); //no vtable lookup
	//	}
	//	api->OGLHgGPUBuffer::Bind(3); //no vtable lookup
	//	glUniform1i(m_uniformLocations[U_BUFFER_OBJECT1], 3);
	//}

	if ((m_uniformLocations[U_TIME_REMAIN] > -1) && (uniforms.remainingTime != nullptr)) {
		glUniform1f(m_uniformLocations[U_TIME_REMAIN], uniforms.remainingTime->seconds());
	}
}