#include <OGL/OGLGpuBuffer.h>
#include <RenderBackend.h>
#include <OGLBackend.h>

std::vector<GLTextureBuffer> OGLHgGPUBuffer::m_useableBufferIds;



GLTextureBuffer::~GLTextureBuffer()
{
	if (buffId > 0) {
		glDeleteBuffers(1, &buffId);
		glDeleteTextures(1, &texId);
	}
	texId = 0;
	buffId = 0;
}

void GLTextureBuffer::AllocateOnGPU()
{
	if (texId == 0) {
		glGenBuffers(1, &buffId);
		glGenTextures(1, &texId);
	}

	if (buffId == 0 || texId == 0)
	{
		fprintf(stderr, "Error creating texture buffer.\n");
	}
}

GLTextureBuffer::GLTextureBuffer(GLTextureBuffer&& rhs)
{
	*this = std::move(rhs);
}

const GLTextureBuffer& GLTextureBuffer::operator=(GLTextureBuffer&& rhs)
{
	buffId = rhs.buffId;
	texId = rhs.texId;

	rhs.buffId = 0;
	rhs.texId = 0;

	m_bufferBound = rhs.m_bufferBound;

	return *this;
}

void GLTextureBuffer::AssociateBuffers()
{
	//setup associate the buffer with the texture
	if (!m_bufferBound)
	{
		auto renderer = (OGLBackend*)RENDERER();
		renderer->BindTexture(10, texId, GL_TEXTURE_BUFFER);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffId);
		m_bufferBound = true;
	}
}



GLTextureBuffer OGLHgGPUBuffer::getGLTextureBuffer()
{
	if (m_useableBufferIds.size() > 0)
	{
		auto r = std::move(m_useableBufferIds.back());
		m_useableBufferIds.pop_back();
		return r;
	}

	GLTextureBuffer ids;
	ids.AllocateOnGPU();
	return ids;
}

void  OGLHgGPUBuffer::releaseGLTextureBuffer(GLTextureBuffer& rhs)
{
	if (rhs.isValid())
	{
		m_useableBufferIds.emplace_back(std::move(rhs));
	}
}


OGLHgGPUBuffer::~OGLHgGPUBuffer()
{
	releaseGLTextureBuffer(m_bufferIds);
}

void OGLHgGPUBuffer::SendToGPU(const IHgGPUBuffer* bufferObject) {
	if (!m_bufferIds.isValid())
	{
		m_bufferIds = getGLTextureBuffer();
	}

	if (m_bufferIds.buffId == 0)
	{
		printf("GL_TEXTURE_BUFFER id is zero! Ignoring\n");
		return;
	}

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

	m_bufferIds.AssociateBuffers();
}

void OGLHgGPUBuffer::Bind(uint16_t textureLocation) {
	auto renderer = (OGLBackend*)RENDERER();
	renderer->BindTexture(3, m_bufferIds.texId, GL_TEXTURE_BUFFER);
}