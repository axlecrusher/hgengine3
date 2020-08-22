#pragma once

#include "quaternion.h"
#include <HgTypes.h>
#include <HgCamera.h>
#include <memory>
#include <string>
#include <unordered_map>

//#include <RenderData.h>
#include <math/MatrixMath.h>

class RenderData;
class Material;
class IHgGPUBuffer;

struct ShaderUniforms
{
	const Material* material;
	const IHgGPUBuffer* gpuBuffer;
	const HgTime* remainingTime;
};

struct shader_source {
	std::string vert_file_path;
	std::string frag_file_path;
	std::string geom_file_path;

	std::string vert_source;
	std::string frag_source;
	std::string geom_source;
};

struct ShaderHandle
{
	ShaderHandle(uint32_t v = 0)
		:value(v)
	{}

	uint32_t value;
};

class HgShader {
	public:
		HgShader()
			:m_uniqueId(0)
		{}

		virtual ~HgShader()
		{};

		virtual void load() = 0;

		//return true if shader compiled successfully. false doesn't necessarily mean error
		virtual bool compile() = 0;
		virtual void destroy() = 0;
		virtual void enable() = 0;

		virtual void setLocalUniforms(const ShaderUniforms& uniforms) = 0;
		virtual void uploadMatrices(const float* worldSpaceMatrix, const HgMath::mat4f& projection, const HgMath::mat4f& view) = 0;

		inline size_t getUniqueId() const { return m_uniqueId; }

		//return the handle of the compiled shader
		inline ShaderHandle getProgramHandle() const { return m_handle; }

		int32_t getAttributeLocation(const std::string& name) const;

		inline shader_source* sourceStruct() const { return m_shaderSource.get(); }

		static HgShader* acquire(const char* vert, const char* frag);

		//increase the shader use count if it exists
		static HgShader* HgShader::acquire(HgShader* shader);

		static void release(HgShader* shader);

		typedef std::unique_ptr<HgShader>(*createShaderCallback)(const char* vert, const char* frag);
		static createShaderCallback Create;
protected:
	inline void setUniqueId(size_t uniqueId) { m_uniqueId = uniqueId; }
	ShaderHandle m_handle;

	//other things not needed often
	std::unique_ptr<shader_source> m_shaderSource;
	std::unordered_map<std::string, uint32_t> m_attribLocations;

private:
	size_t m_uniqueId;
};