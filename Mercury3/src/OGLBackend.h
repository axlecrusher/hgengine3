#pragma once

#include <RenderBackend.h>

class OGLBackend : public RenderBackend {
public:
	OGLBackend() {
		m_type = RendererType::OPENGL;
	}
	static void Init();

	void Clear();
	void BeginFrame();
	void EndFrame();

};

extern OGLBackend oglRenderer;