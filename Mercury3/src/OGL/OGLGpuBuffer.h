#pragma once

#include <OGLheaders.h>
#include <HgVboMemory.h>
#include <HgGPUBuffer.h>
#include <GLBuffer.h>

//Contains opengl buffer and texture id for texture buffers
struct GLTextureBuffer
{
	GLTextureBuffer() : buffId(0), texId(0), m_bufferBound(false)
	{}

	~GLTextureBuffer();

	void AllocateOnGPU();

	//Only allow move operators.
	//We don't want copies because we don't want the destructor to deallocate the opengl buffer ids.
	GLTextureBuffer(const GLTextureBuffer& rhs) = delete;
	GLTextureBuffer(GLTextureBuffer&& rhs);

	const GLTextureBuffer& operator=(const GLTextureBuffer& rhs) = delete;
	const GLTextureBuffer& operator=(GLTextureBuffer&& rhs);

	void AssociateBuffers();

	bool isValid() const { return buffId != 0; }

	GLuint buffId;
	GLuint texId;
	bool m_bufferBound;
};

class OGLHgGPUBuffer : public IGPUBufferImpl {
public:
	OGLHgGPUBuffer() : m_lastSize(0) {}
	~OGLHgGPUBuffer();

	virtual void SendToGPU(const IHgGPUBuffer* bufferObject) final;
	virtual void Bind(uint16_t textureLocation) final;

	//inline GLuint TextureId() const { return m_bufferIds.texId; }
private:
	static GLTextureBuffer getGLTextureBuffer();

	//return GLTextureBuffer to pool for reuse
	static void releaseGLTextureBuffer(GLTextureBuffer& rhs);
	static std::vector<GLTextureBuffer> m_useableBufferIds; //a pool of open glbuffer resources

	GLTextureBuffer m_bufferIds;

	size_t m_lastSize;
};