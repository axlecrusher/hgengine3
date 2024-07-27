#include <OGLFramebuffer.h>

OGLFramebuffer::OGLFramebuffer()
	: m_depthBufferId(0), m_renderTextureId(0), m_renderFramebufferId(0),
	m_resolveTextureId(0), m_resolveFramebufferId(0)
{

}

OGLFramebuffer::~OGLFramebuffer()
{
	if (m_depthBufferId>0) glDeleteRenderbuffers(1, &m_depthBufferId);
	if (m_renderTextureId > 0) glDeleteTextures(1, &m_renderTextureId);
	if (m_renderFramebufferId > 0) glDeleteFramebuffers(1, &m_renderFramebufferId);
	if (m_resolveTextureId > 0) glDeleteTextures(1, &m_resolveTextureId);
	if (m_resolveFramebufferId > 0) glDeleteFramebuffers(1, &m_resolveFramebufferId);
}

bool OGLFramebuffer::Init(uint16_t width, uint16_t height)
{
	m_width = width;
	m_height = height;

	glGenFramebuffers(1, &m_renderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_renderFramebufferId);

	glGenRenderbuffers(1, &m_depthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferId);
	//glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT32F, width, height); //floating point depth buffer works better for reversed depth buffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferId);

	glGenTextures(1, &m_renderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_renderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, width, height, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_renderTextureId, 0);

	glGenFramebuffers(1, &m_resolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_resolveFramebufferId);

	glGenTextures(1, &m_resolveTextureId);
	glBindTexture(GL_TEXTURE_2D, m_resolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void OGLFramebuffer::Enable()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_renderFramebufferId);
	glViewport(0, 0, m_width, m_height);
}

void OGLFramebuffer::Disable()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OGLFramebuffer::Copy()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_renderFramebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolveFramebufferId);

	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}