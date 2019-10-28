#pragma once

#include <stdint.h>
#include <vector>

#include <HgEntity.h>
#include <IFramebuffer.h>

enum RendererType {
	OPENGL = 0,
	VULKAN,
	DIRECTX
};

struct Viewport
{
	Viewport()
		:x(0), y(0), width(0), height(0)
	{}

	bool operator==(const Viewport& rhs) const
	{
		return (x == rhs.x) && (y == rhs.y) && (width == rhs.width)
			&& (height == rhs.height);
	}

	uint16_t x, y;
	uint16_t width, height;
};

class RenderBackend {
public:

	virtual void Init() = 0;
	virtual void Clear() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;
	virtual void setRenderAttributes(BlendMode blendMode, RenderData::Flags flags) = 0;
	virtual void setViewport(const Viewport& vp) = 0;

	virtual std::unique_ptr<IFramebuffer> CreateFrameBuffer() = 0;

	RendererType Type() const { return m_type; }
	//void setup_viewports(uint16_t width, uint16_t height);

protected:
	RendererType m_type;

	Viewport m_currentViewport;
	//struct viewport view_port[3];
	//uint8_t _currenViewPort_idx = 0xFF;
};

struct RenderInstance {
	RenderInstance(const HgMath::mat4f& _worldSpaceMatrix, RenderDataPtr& rd, int8_t order = 0)
		:renderData(rd), drawOrder(order)
	{
		_worldSpaceMatrix.store(worldSpaceMatrix);
	}
	float worldSpaceMatrix[16];
	RenderDataPtr renderData;
	int8_t drawOrder;
};

class RenderQueue
{
public:
	void Enqueue(const HgEntity* entity);
	void Enqueue(RenderDataPtr& rd);

	//sorts queue on draw order for proper rendering
	void Finalize();

	//Clears existing queue
	void Clear()
	{
		m_opaqueEntities.clear();
		m_transparentEntities.clear();
	}

	const std::vector<RenderInstance>& getOpaqueQueue() const { return m_opaqueEntities; }
	const std::vector<RenderInstance>& getTransparentQueue() const { return m_transparentEntities; }
private:
	void Enqueue(RenderDataPtr& rd, const HgMath::mat4f& wsm, int8_t drawOrder);
	void sort(std::vector<RenderInstance>& v);

	std::vector<RenderInstance> m_opaqueEntities;
	std::vector<RenderInstance> m_transparentEntities;
};

class HgCamera;

namespace Renderer {

	//Must happen after window is created;
	void Init();

	void Render(const Viewport& vp, const HgMath::mat4f& viewMatrix, const HgMath::mat4f& projection, RenderQueue* queue);
}

RenderBackend* RENDERER();
