#pragma once

#include <stdint.h>
#include <GLBuffer.h>

#include <core/Instancing.h>
#include <../oglShaders.h>

#include <HgGPUBuffer.h>
#include <GpuBufferId.h>

#include <Enumerations.h>

struct SetupParams; // Forward declare

namespace VertexAttributeTypes
{
	//Map attribute buffer to attribute location in the shader
	void MapToAttributeLocation(const SetupParams& p, int vCount);

	struct vec4
	{
		float xyzw[4]; //define so sizeof() can be used

		static void Setup(const SetupParams& p)
		{
			MapToAttributeLocation(p, 1);
		}
	};

	struct dvec4
	{
		//double xyzw[4]; //define so sizeof() can be used
	};

	struct mat4
	{
		float xyzw[16]; //define so sizeof() can be used

		static void Setup(const SetupParams& p)
		{
			MapToAttributeLocation(p, 4);
		}
	};
}

void InvalidAttributeError(const HgShader* shader, const std::string* name);

class GLVertexAttributeBuffer
{
public:
	GLVertexAttributeBuffer()
		:m_maxSize(0)
	{}

	//Allocate buffer in opengl
	bool Init()
	{
		return bufferId.Init();
	}

	template<typename T>
	inline void SendDataToGPU(T* data, unsigned int count)
	{
		toGPU(data, sizeof(T) * count);
	}

	auto getValue() const { return bufferId.value; }

	void AllocateOnGPU(size_t sizeBytes, BUFFER_USE_TYPE useType);
	//void toGPU(const void* data, const size_t byteCount);

	//get pointer to gpu memory
	virtual MappedMemory getGPUMemoryPtr() { MappedMemory r; return r; };
	virtual void ReleaseMappedMemory(MappedMemory* mm) {};

private:
	GpuBufferId bufferId;
	size_t m_maxSize;
};

namespace GraphicsDriverFunctions
{
	//lame cop out for things that do opengl

	void ReleaseMappedMemory(MappedMemory* mm);
	MappedMemory getGPUMemoryPtr(IGPUBuffer* gpuBuffer, GLVertexAttributeBuffer* vab);
}

struct SetupParams
{
	IGPUBuffer* iglbuffer;
	GLVertexAttributeBuffer* attribBuffer;
	std::string* attributeName;
	const Instancing::InstancingMetaData* imd;
	const HgShader* shader;
};

template<typename T>
//class VertexAttributeBuffer : public IGLBufferUse
class VertexAttributeBuffer : public IGPUBuffer
{
public:
	VertexAttributeBuffer(const std::string attributeName)
		:m_attributeName(attributeName)
	{
		setUseType(BUFFER_DRAW_STREAM);
	}

	//Allocates space for count instances of T on the GPU
	void AllocateCountOnGPU(uint32_t count)
	{
		VertexAttributeBuffer<T>::AllocateOnGPU(sizeof(T) * count);
	}

	virtual void AllocateOnGPU(size_t sizeBytes)
	{
		m_attributeBuffer.AllocateOnGPU(sizeBytes, getUseType());
	}

	virtual void Setup(const Instancing::InstancingMetaData& imd, const HgShader& shader)
	{
		SetupParams p;
		p.iglbuffer = this;
		p.attribBuffer = &m_attributeBuffer;
		p.attributeName = &m_attributeName;
		p.imd = &imd;
		p.shader = &shader;
		T::Setup(p);
	}

	void setAttributeName(const std::string& name) { m_attributeName = name; }

	virtual MappedMemory getGPUMemoryPtr()
	{
		return GraphicsDriverFunctions::getGPUMemoryPtr(this, &m_attributeBuffer);
	}

	virtual void ReleaseMappedMemory(MappedMemory* mm)
	{
		GraphicsDriverFunctions::ReleaseMappedMemory(mm);
	}

private:
	GLVertexAttributeBuffer m_attributeBuffer;
	std::string m_attributeName;
};

using vec4VertexAttribute = VertexAttributeBuffer<VertexAttributeTypes::vec4>;
using MatrixVertexAttribute = VertexAttributeBuffer<VertexAttributeTypes::mat4>;