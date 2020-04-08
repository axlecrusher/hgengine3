#pragma once

#include <glew.h>
#include <stdint.h>
#include <GLBuffer.h>

#include <core/Instancing.h>
#include <../oglShaders.h>

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

	void toGPU(const void* data, const size_t byteCount);

private:

	GLBufferId bufferId;
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

	virtual void Setup(const Instancing::InstancingMetaData& imd, const HgShader& shader);

	void setAttributeName(const std::string& name) { m_attributeName = name; }

private:
	GLVertexAttributeBuffer m_attributeBuffer;
	std::string m_attributeName;
};