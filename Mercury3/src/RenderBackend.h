#pragma once

class RenderBackend {
public:

	virtual void Clear() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

};

namespace Renderer {
	void InitOpenGL();
}

extern RenderBackend* RENDERER;
