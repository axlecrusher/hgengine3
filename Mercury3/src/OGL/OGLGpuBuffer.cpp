#include <OGL/OGLGpuBuffer.h>

std::vector<GLBufferId> OGLHgGPUBuffer::m_useableBufferIds;


GLBufferId::~GLBufferId()
{
	if (buffId > 0) {
		glDeleteBuffers(1, &buffId);
		glDeleteTextures(1, &texId);
	}
	texId = 0;
	buffId = 0;
}

void GLBufferId::AllocateOnGPU()
{
	if (texId == 0) {
		glGenBuffers(1, &buffId);
		glGenTextures(1, &texId);
	}
}

GLBufferId::GLBufferId(GLBufferId&& rhs)
{
	*this = std::move(rhs);
}

const GLBufferId& GLBufferId::operator=(GLBufferId&& rhs)
{
	std::swap(buffId, rhs.buffId);
	std::swap(texId, rhs.texId);
	return *this;
}



GLBufferId OGLHgGPUBuffer::getGLBufferId()
{
	if (m_useableBufferIds.size() > 0)
	{
		auto r = std::move(m_useableBufferIds.back());
		m_useableBufferIds.pop_back();
		return r;
	}

	GLBufferId ids;
	ids.AllocateOnGPU();
	return ids;
}

void  OGLHgGPUBuffer::freeGLBufferId(GLBufferId& rhs)
{
	m_useableBufferIds.emplace_back(std::move(rhs));
}


OGLHgGPUBuffer::~OGLHgGPUBuffer()
{
	freeGLBufferId(m_bufferIds);
}

void OGLHgGPUBuffer::SendToGPU(const IHgGPUBuffer* bufferObject) {
	m_bufferIds = getGLBufferId();

	glBindBuffer(GL_TEXTURE_BUFFER, m_bufferIds.buffId);

	const size_t size = bufferObject->sizeBytes();

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
	glBindTexture(GL_TEXTURE_BUFFER, m_bufferIds.texId);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_bufferIds.buffId);
}