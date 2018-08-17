#include <OGL/OGLGpuBuffer.h>

OGLHgGPUBuffer::~OGLHgGPUBuffer() {
	if (m_buffId > 0) {
		glDeleteBuffers(1, &m_buffId);
		glDeleteTextures(1, &m_texId);
	}
	m_texId = 0;
	m_buffId = 0;
}

void OGLHgGPUBuffer::SendToGPU(const IHgGPUBuffer* bufferObject) {
	if (m_texId == 0) {
		glGenBuffers(1, &m_buffId);
		glGenTextures(1, &m_texId);
	}

	glBindBuffer(GL_TEXTURE_BUFFER, m_buffId);

	const size_t size = bufferObject->size();

	if (m_lastSize < size) {
		//grow buffer and load all data
		glBufferData(GL_TEXTURE_BUFFER, size, bufferObject->getBufferPtr(), GL_DYNAMIC_DRAW);
	}
	else {
		//load all data
		glBufferSubData(GL_TEXTURE_BUFFER, 0, size, bufferObject->getBufferPtr());
	}
	m_lastSize = size;
}

void OGLHgGPUBuffer::Bind() {
	glBindTexture(GL_TEXTURE_BUFFER, m_texId);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_buffId);
}