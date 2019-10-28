#pragma once

#include <RenderBackend.h>
#include <glew.h>

//Class representing the OpenGL texture unit being modified
class OGLTextureUnit
{
public:
	OGLTextureUnit()
	{
		memset(&m_targets, 0, sizeof(m_targets));
	}
	void BindTexture(GLenum target, GLuint textureId);
	GLuint GetBoundTexture(GLenum target);
private:
	static uint8_t convertTextureTarget(GLenum target);

	GLuint m_targets[3] = { 0 };
};

class OGLBackend : public RenderBackend {
public:
	OGLBackend() {
		m_type = RendererType::OPENGL;
	}

	//create graphics api singleton
	static RenderBackend* Create();

	void Init();

	void Clear();
	void BeginFrame();
	void EndFrame();
	void setViewport(const Viewport& vp);
	std::unique_ptr<IFramebuffer> CreateFrameBuffer();

	OGLTextureUnit& ActiveTexture(uint16_t t);
	void BindTexture(uint16_t textureLocation, HgTexture::Handle textureHandle, GLenum target);

	enum EnabledState
	{
		UNKNOWN,
		ENABLED,
		DISABLED
	};

	void setRenderAttributes(BlendMode blendMode, RenderData::Flags flags);
};

extern OGLBackend oglRenderer;