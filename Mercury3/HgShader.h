#pragma once

#include "quaternion.h"
#include <HgTypes.h>
#include <HgCamera.h>
#include <memory>

//#include <RenderData.h>
#include <math/matrix.h>

class RenderData;

class HgShader {
	public:
		HgShader()
			:m_uniqueId(0)
		{}

		virtual ~HgShader()
		{};

		virtual void load() = 0;
		virtual void destroy() = 0;
		virtual void enable() = 0;

		virtual void setLocalUniforms(const RenderData& rd) = 0;
		virtual void uploadMatrices(const float* worldSpaceMatrix, const HgMath::mat4f& projection, const HgMath::mat4f& view) = 0;

		inline size_t getUniqueId() const { return m_uniqueId; }

		static HgShader* acquire(const char* vert, const char* frag);

		//increase the shader use count if it exists
		static HgShader* HgShader::acquire(HgShader* shader);

		static void release(HgShader* shader);

		typedef std::unique_ptr<HgShader>(*createShaderCallback)(const char* vert, const char* frag);
		static createShaderCallback Create;
protected:
	inline void setUniqueId(size_t uniqueId) { m_uniqueId = uniqueId; }
private:
	size_t m_uniqueId;
};