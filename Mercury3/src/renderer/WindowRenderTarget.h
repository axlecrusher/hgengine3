#pragma once

#include <IRenderTarget.h>
#include <RenderBackend.h>

class WindowRenderTarget : public IRenderTarget
{
public:
	WindowRenderTarget();

	~WindowRenderTarget()
	{}

	virtual bool Init();
	virtual void Render(HgCamera* camera, RenderQueue* queue);

private:
	HgMath::mat4f m_projection;
	Viewport m_windowViewport;
};