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
public:
	ShaderHandle(uint32_t v = 0)
		:m_value(v)
	{}

	operator uint32_t() const { return m_value; }
private:
	uint32_t m_value;
};

class IShaderImpl {
	public:
		using attributeNameType = std::hash<std::string>::result_type;

		IShaderImpl()
			:m_uniqueId(0)
		{}

		virtual ~IShaderImpl() = default;

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

		int32_t getAttributeLocation(attributeNameType name) const;

		inline shader_source* sourceStruct() const { return m_shaderSource.get(); }

		inline void setFragmentPath(std::string& p) { frag_path = p; }
		inline const std::string& getFragmentPath() const { return frag_path; }

		inline void setVertexPath(std::string& p) { vertex_path = p; }
		inline const std::string& getVertexPath() const { return vertex_path; }

protected:
	inline void setUniqueId(size_t uniqueId) { m_uniqueId = uniqueId; }
	ShaderHandle m_handle;

	//other things not needed often
	std::unique_ptr<shader_source> m_shaderSource;
	//std::unordered_map<std::string, uint32_t> m_attribLocations;
	std::vector< std::pair<attributeNameType, int32_t> > m_attribLocations;
	std::string vertex_path, frag_path;

private:
	size_t m_uniqueId;
};

class HgShader {
public:
	HgShader() = default;

	HgShader(const HgShader& other) = default;

	HgShader(const std::shared_ptr<IShaderImpl> impl)
		:m_impl(impl)
	{}

	inline IShaderImpl* getImplementation() const {
		return m_impl.get();
	}

	inline int32_t getAttributeLocation(IShaderImpl::attributeNameType name) const
	{
		return getImplementation()->getAttributeLocation(name);
	}

	inline shader_source* sourceStruct() const { return getImplementation()->sourceStruct(); }

	void enable() { getImplementation()->enable(); }

	bool compile() { return getImplementation()->compile(); }

	void setLocalUniforms(const ShaderUniforms& uniforms) { getImplementation()->setLocalUniforms(uniforms); }
	void uploadMatrices(const float* worldSpaceMatrix, const HgMath::mat4f& projection, const HgMath::mat4f& view)
	{
		getImplementation()->uploadMatrices(worldSpaceMatrix, projection, view);
	}

	static HgShader acquire(const char* vert, const char* frag);

	//increase the shader use count if it exists
	static HgShader HgShader::acquire(const HgShader& shader);

	typedef std::shared_ptr<IShaderImpl>(*createShaderCallback)(const char* vert, const char* frag);
	static createShaderCallback Create;

private:
	std::shared_ptr<IShaderImpl> m_impl;
};