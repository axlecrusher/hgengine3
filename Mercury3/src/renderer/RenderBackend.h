#pragma once

#include <stdint.h>
#include <vector>

#include <HgEntity.h>
#include <IFramebuffer.h>
#include <IRenderTarget.h>
#include <core/Instancing.h>

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

//make a viewport look like a render target. For use with getting projection matrices
class ViewportRenderTarget : public IRenderTarget
{
public:
	ViewportRenderTarget(const Viewport* vp)
		:m_vp(vp)
	{}

	virtual bool Init() { return true; };
	virtual void Render(const RenderParamsList& l) {};

	virtual HgMath::mat4f getPerspectiveMatrix() const;
	virtual HgMath::mat4f getOrthoMatrix() const;

	virtual uint32_t getWidth() const
	{
		return m_vp->width;
	}

	virtual uint32_t getHeight() const
	{
		return m_vp->height;
	}

private:
	const Viewport* m_vp;
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
	RenderInstance(const HgMath::mat4f& _worldSpaceMatrix, RenderDataPtr& rd, const vector3f& _velocityVector, int8_t order = 0, const Instancing::InstancingMetaData* imdPtr = nullptr)
		:renderData(rd), drawOrder(order), velocityVector(_velocityVector)
	{
		_worldSpaceMatrix.store(worldSpaceMatrix);
		_worldSpaceMatrix.store(interpolatedWorldSpaceMatrix);

		if (imdPtr)
		{
			imd = *imdPtr;
		}
	}

	Instancing::InstancingMetaData imd;

	float worldSpaceMatrix[16];
	float interpolatedWorldSpaceMatrix[16];
	RenderDataPtr renderData;
	int8_t drawOrder;
	HgTime remainingTime;

	vector3f velocityVector;
};

class RenderQueue
{
public:
	void Enqueue(HgEntity* entity, HgTime t);
	void Enqueue(RenderDataPtr& rd);
	void Enqueue(const Instancing::InstancingMetaData& imd);

	//sorts queue on draw order for proper rendering
	void Finalize(const HgTime& remain);

	//Clears existing queue
	void Clear()
	{
		m_opaqueEntities.clear();
		m_transparentEntities.clear();
	}

	const std::vector<RenderInstance>& getOpaqueQueue() const { return m_opaqueEntities; }
	const std::vector<RenderInstance>& getTransparentQueue() const { return m_transparentEntities; }
private:
	void Enqueue(RenderDataPtr& rd, const HgMath::mat4f& wsm, int8_t drawOrder, const vector3f& velocity, const Instancing::InstancingMetaData* imd = nullptr);

	//Lower draw order is first
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
