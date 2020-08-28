#pragma once

#include <glew.h>
#include <stdint.h>
#include <GLBuffer.h>

#include <core/Instancing.h>
#include <../oglShaders.h>

#include <HgGPUBuffer.h>
#include <GpuBufferId.h>

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

	void AllocateOnGPU(size_t sizeBytes);
	void toGPU(const void* data, const size_t byteCount);

	//get pointer to gpu memory
	virtual MappedMemory getGPUMemoryPtr() { MappedMemory r; return r; };
	virtual void ReleaseMappedMemory(MappedMemory* mm) {};

private:
	GpuBufferId bufferId;
	size_t m_maxSize;
};

class MatrixVertexAttribute : public IGLBufferUse
{
public:
	MatrixVertexAttribute(const std::string attributeName)
		:m_attributeName(attributeName)
	{}

	~MatrixVertexAttribute()
	{
	}

	virtual void SendToGPU(const IHgGPUBuffer* bufferObject);
	virtual void AllocateOnGPU(size_t sizeBytes) { m_attributeBuffer.AllocateOnGPU(sizeBytes); }
	virtual void Setup(const Instancing::InstancingMetaData& imd, const HgShader& shader);

	void setAttributeName(const std::string& name) { m_attributeName = name; }

	virtual MappedMemory getGPUMemoryPtr();
	virtual void ReleaseMappedMemory(MappedMemory* mm);

private:

	GLVertexAttributeBuffer m_attributeBuffer;
	std::string m_attributeName;
};