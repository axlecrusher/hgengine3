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

	virtual void Init() = 0;
	virtual void Clear() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;
	virtual void setRenderAttributes(BlendMode blendMode, RenderData::Flags flags) = 0;
	virtual void Viewport(uint8_t idx) = 0;

	RendererType Type() const { return m_type; }
	void setup_viewports(uint16_t width, uint16_t height);

protected:
	RendererType m_type;

	struct viewport view_port[3];
	uint8_t _currenViewPort_idx = 0xFF;
};

struct RenderInstance {
	RenderInstance(const HgMath::mat4f& _worldSpaceMatrix, RenderDataPtr& rd)
		:renderData(rd)
	{
		_worldSpaceMatrix.store(worldSpaceMatrix);
	}
	float worldSpaceMatrix[16];
	RenderDataPtr renderData;
};

class HgCamera;

namespace Renderer {

	//Must happen after window is created;
	void Init();

	extern std::vector<RenderInstance> opaqueEntities;
	extern std::vector<RenderInstance> transparentEntities;

	void Render(uint8_t stereo_view, HgCamera* camera, const HgMath::mat4f& projection);
	void Enqueue(HgEntity& e);

	extern HgMath::mat4f ProjectionMatrix;
	extern HgMath::mat4f ViewMatrix;
}

RenderBackend* RENDERER();
