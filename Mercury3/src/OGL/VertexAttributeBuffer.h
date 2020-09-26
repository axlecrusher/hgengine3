#pragma once

#include <stdint.h>
#include <GLBuffer.h>

//#include <core/Instancing.h>
#include <../oglShaders.h>

#include <HgGPUBuffer.h>
#include <GpuBufferId.h>

#include <Enumerations.h>

//Forward declarations
struct SetupParams;
namespace Instancing
{
	struct InstancingMetaData;
}

namespace VertexAttributeTypes
{
	//Map attribute buffer to attribute location in the shader
	void MapToAttributeLocation(const SetupParams& p, int vCount);

	struct vec4
	{
		union xyzw
		{
			struct
			{
				float r, g, b, a;
			} color;
			float xyzw[4]; //define so sizeof() can be used
		} xyzw;

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

	inline size_t size() const { return m_maxSize; }

private:
	GpuBufferId bufferId;
	size_t m_maxSize;
};

namespace GraphicsDriverFunctions
{
	//lame cop out for things that do opengl

	void ReleaseMappedMemory(GLVertexAttributeBuffer* vab);
	MappedMemory getGPUMemoryPtr(IGPUBuffer* gpuBuffer, GLVertexAttributeBuffer* vab);
}

struct GPUBufferMapSettings
{
	GPUBufferMapSettings()
		:gpuBuffer(nullptr), byteOffset(0)
	{}

	IGPUBuffer* gpuBuffer;
	uint32_t byteOffset;
};

struct SetupParams
{
	GPUBufferMapSettings bufferSettings;
	GLVertexAttributeBuffer* attribBuffer;
	std::string* attributeName;
	const HgShader* shader;
};

#define BUFSIZE 1

template<typename T>
//class VertexAttributeBuffer : public IGLBufferUse
class VertexAttributeBuffer : public IGPUBuffer
{
public:
	VertexAttributeBuffer(const std::string attributeName)
		:m_attributeName(attributeName)
	{
		setUseType(BUFFER_STREAM_DRAW);
		m_bufferIdx = 0;
	}

	//Allocates space for count instances of T on the GPU
	void AllocateCountOnGPU(uint32_t count)
	{
		VertexAttributeBuffer<T>::AllocateOnGPU(sizeof(T) * count);
	}

	virtual void AllocateOnGPU(size_t sizeBytes)
	{
		for (int i = 0; i < BUFSIZE; i++)
		{
			m_attributeBuffer[i].AllocateOnGPU(sizeBytes, getUseType());
		}
	}

	virtual void Setup(const GPUBufferMapSettings* settings, const HgShader& shader)
	{
		SetupParams p;
		p.bufferSettings = *settings;
		p.attribBuffer = getAttribBuffer();
		p.attributeName = &m_attributeName;
		p.shader = &shader;
		T::Setup(p);
	}

	void setAttributeName(const std::string& name) { m_attributeName = name; }

	virtual MappedMemory getGPUMemoryPtr()
	{
		return GraphicsDriverFunctions::getGPUMemoryPtr(this, getAttribBuffer());
	}

	virtual void ReleaseMappedMemory(MappedMemory* mm)
	{
		GraphicsDriverFunctions::ReleaseMappedMemory( getAttribBuffer() );
	}

	void SwapBuffer() { m_bufferIdx = (m_bufferIdx + 1) % BUFSIZE; }

private:
	inline GLVertexAttributeBuffer* getAttribBuffer() { return &m_attributeBuffer[m_bufferIdx]; }

	GLVertexAttributeBuffer m_attributeBuffer[BUFSIZE];
	uint8_t m_bufferIdx;
	std::string m_attributeName;
};

using vec4VertexAttribute = VertexAttributeBuffer<VertexAttributeTypes::vec4>;
using MatrixVertexAttribute = VertexAttributeBuffer<VertexAttributeTypes::mat4>;