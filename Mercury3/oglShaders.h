#pragma once

#include <glew.h>
#include <stdint.h>

#include <HgShader.h>
#include <oglDisplay.h>

struct shader_source {
	char* vert_file_path;
	char* frag_file_path;
	char* geom_file_path;

	char* vert_source;
	char* frag_source;
	char* geom_source;
};

class HgOglShader : public HgShader {
	public:
		HgOglShader();

		virtual void load();
		virtual void destroy();
		virtual void enable();

		void setProgramCode(shader_source* ss) { program_code = ss; }

		/* Perhaps shader uniforms should be stored locally per instance of a shader and then
		sent to the video driver when the shader instance is enables.
		Currently shaders are essentially global and shared across any element that needs a specific shader.
		We could make the shader binary shared but keep unform variables on a per instance basis and load
		them as we process through different instances.
		*/

		virtual void setGlobalUniforms(const HgCamera& c);
		virtual void setLocalUniforms(const quaternion* rotation, const point* position, float scale, const point* origin);

		int8_t uniform_locations[U_UNIFORM_COUNT];
	private:
		static void setup_shader(HgOglShader* s);

		GLuint program_id;
		uint8_t source_loaded;

		//other things not needed often
		shader_source* program_code;
};

//GLuint shaders_load(const char* path, uint32_t shader_type);
//void useShaderProgram(GLuint id);
//void _print_programme_info_log(GLuint programme);
//void _print_shader_info_log(GLuint idx);

HgShader* HGShader_ogl_create(const char* vert, const char* frag);