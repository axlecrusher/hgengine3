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
	void Viewport(uint8_t idx);

	void setRenderAttributes(BlendMode blendMode, RenderFlags flags);
};

extern OGLBackend oglRenderer;