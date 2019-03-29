#pragma once

#include <IRenderTarget.h>

class WindowRenderTarget : public IRenderTarget
{
public:
	WindowRenderTarget();

	~WindowRenderTarget()
	{}

	virtual void Init();
	virtual void Render(HgCamera* camera, RenderQueue* queue);

private:
	HgMath::mat4f m_projection;
};