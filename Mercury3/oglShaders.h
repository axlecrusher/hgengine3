#pragma once

#include <glew.h>
#include <stdint.h>

#include <HgShader.h>
#include <oglDisplay.h>

#include <quaternion_dual.h>

//Conversion struct to convert between Hg ShaderHandle and OPENGL GLuint shader handle
struct OGLShaderHandle
{
	OGLShaderHandle(ShaderHandle h)
		:value(h.value)
	{}

	OGLShaderHandle(GLuint h)
		:value(h)
	{}

	operator GLuint() const { return value; }
	operator ShaderHandle() const { return ShaderHandle(value); }

	GLuint value;
};

class HgOglShader : public HgShader {
	public:
		HgOglShader();
		virtual ~HgOglShader();

		virtual void load();
		virtual bool compile();
		virtual void destroy();
		virtual void enable();

		inline void setProgramCode(std::unique_ptr<shader_source>& ss) { m_shaderSource = std::move(ss); }

		virtual void setLocalUniforms(const ShaderUniforms& uniforms);
		virtual void uploadMatrices(const float* worldSpaceMatrix, const HgMath::mat4f& projection, const HgMath::mat4f& view);

		static std::unique_ptr<HgShader> Create(const char* vert, const char* frag);
	private:
		enum class LoadState: uint8_t{
			NOT_LOADED=0,
			SOURCE_LOADED,
			READY
		};

		static void setup_shader(HgOglShader* s);
		void sendLocalUniformsToGPU(const ShaderUniforms& uniforms);

		/* Perhaps shader uniforms should be stored locally per instance of a shader and then
		sent to the video driver when the shader instance is enabled.
		Currently shaders are essentially global and shared across any entity that needs a specific shader.
		We could make the shader binary shared but keep unform variables on a per instance basis and load
		them as we process through different instances.

		What would the benefit of this be? I can't remember. Maybe it makes sense for local uniforms.
		*/

		//GLuint program_id;
		GLint m_uniformLocations[U_UNIFORM_COUNT];
		LoadState m_loadState;
};
