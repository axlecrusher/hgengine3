#pragma once

#include <RenderBackend.h>

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

	enum EnabledState
	{
		UNKNOWN,
		ENABLED,
		DISABLED
	};

	void setRenderAttributes(BlendMode blendMode, RenderData::Flags flags);
};

extern OGLBackend oglRenderer;