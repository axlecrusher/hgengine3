
#pragma once

#include <stdint.h>
#include <vector>

#include <HgElement.h>

enum RendererType {
	OPENGL = 0,
	VULKAN,
	DIRECTX
};

struct viewport {
	uint16_t x, y, width, height;
};

class RenderBackend {
public:

	virtual void Clear() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

	inline RendererType Type() const { return m_type; }
	virtual void Viewport(uint8_t idx) = 0;

	void setup_viewports(uint16_t width, uint16_t height);
protected:
	RendererType m_type;

	struct viewport view_port[3];
	uint8_t _currenViewPort_idx = 0xFF;
};

//class HgElement;
class HgCamera;

//#include <HgElement.h>

namespace Renderer {
	void InitOpenGL();

	extern std::vector<HgElement*> opaqueElements;
	extern std::vector<HgElement*> transparentElements;

	void Render(uint8_t stereo_view, HgCamera* camera, const HgMath::mat4f& projection);
	inline void Enqueue(HgElement& e) {
		if (e.flags.transparent) {
			Renderer::transparentElements.push_back(&e);
		}
		else {
			Renderer::opaqueElements.push_back(&e);
		}
	}

	extern HgMath::mat4f projection_matrix;
	extern HgMath::mat4f view_matrix;
}

extern RenderBackend* RENDERER;
