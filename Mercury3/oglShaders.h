#pragma once

#include <glew.h>
#include <stdint.h>

#include <HgShader.h>
#include <oglDisplay.h>

struct shader_source {
	std::string vert_file_path;
	std::string frag_file_path;
	std::string geom_file_path;

	std::string vert_source;
	std::string frag_source;
	std::string geom_source;
};

class HgOglShader : public HgShader {
	public:
		HgOglShader();
		virtual ~HgOglShader();

		virtual void load();
		virtual void destroy();
		virtual void enable();

		inline void setProgramCode(std::unique_ptr<shader_source>& ss) { m_shaderSource = std::move(ss); }

		/* Perhaps shader uniforms should be stored locally per instance of a shader and then
		sent to the video driver when the shader instance is enables.
		Currently shaders are essentially global and shared across any element that needs a specific shader.
		We could make the shader binary shared but keep unform variables on a per instance basis and load
		them as we process through different instances.
		*/

		virtual void setGlobalUniforms(const HgCamera& c);
		virtual void setLocalUniforms(const quaternion* rotation, const point* position, float scale, const point* origin, const RenderData* rd);

		static std::unique_ptr<HgShader> Create(const char* vert, const char* frag);
	private:
		enum class LoadState: uint8_t{
			NOT_LOADED=0,
			SOURCE_LOADED,
			READY
		};

		static void setup_shader(HgOglShader* s);
		void sendGlobalUniformsToGPU(const HgCamera& c);
		void sendLocalUniformsToGPU(const quaternion* rotation, const point* position, float scale, const point* origin, const RenderData* rd);

		GLuint program_id;
		GLint m_uniformLocations[U_UNIFORM_COUNT];
		LoadState m_loadState;

		//other things not needed often
		std::unique_ptr<shader_source> m_shaderSource;
};

//GLuint shaders_load(const char* path, uint32_t shader_type);
//void useShaderProgram(GLuint id);
//void _print_programme_info_log(GLuint programme);
//void _print_shader_info_log(GLuint idx);