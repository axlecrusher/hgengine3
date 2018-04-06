#pragma once

enum RendererType {
	OPENGL = 0,
	VULKAN,
	DIRECTX
};

class RenderBackend {
public:

	virtual void Clear() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

	inline RendererType Type() const { return m_type; }
protected:
	RendererType m_type;
};

namespace Renderer {
	void InitOpenGL();
}

extern RenderBackend* RENDERER;
