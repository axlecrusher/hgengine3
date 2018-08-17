#pragma once

#include <glew.h>
#include <HgVboMemory.h>
#include <HgGPUBuffer.h>

class OGLHgGPUBuffer : public IGPUBufferImpl {
public:
	OGLHgGPUBuffer() : m_buffId(0), m_texId(0), m_lastSize(0) {}
	~OGLHgGPUBuffer();

	virtual void SendToGPU(const IHgGPUBuffer* bufferObject) final;
	virtual void Bind() final;

	inline GLuint TextureId() const { return m_texId; }
private:
	GLuint m_buffId;
	GLuint m_texId;

	size_t m_lastSize;
};