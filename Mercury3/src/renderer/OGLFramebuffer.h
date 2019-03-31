#pragma once
#include <IFramebuffer.h>
#include <glew.h>

class OGLFramebuffer : public IFramebuffer
{
public:
	OGLFramebuffer();
	virtual ~OGLFramebuffer();

	virtual bool Init(uint16_t width, uint16_t height);
	virtual void Enable();
	virtual void Disable();
	
	void Copy();

	GLuint getRenderFramebufferID() const { return m_renderFramebufferId; }
	GLuint getRenderTextureID() const { return m_renderTextureId; }

	GLuint getResolveFramebufferID() const { return m_resolveFramebufferId; }
	GLuint getResolveTextureID() const { return m_resolveTextureId; }
private:
	uint16_t m_width, m_height;

	GLuint m_depthBufferId;
	GLuint m_renderTextureId;
	GLuint m_renderFramebufferId;
	GLuint m_resolveTextureId;
	GLuint m_resolveFramebufferId;
};