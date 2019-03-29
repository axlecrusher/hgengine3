#pragma once

#include <IRenderTarget.h>

class StereoCrossEye : public IRenderTarget
{
public:
	StereoCrossEye();

	~StereoCrossEye()
	{}

	virtual void Init();
	virtual void Render(HgCamera* camera, RenderQueue* queue);

private:
	HgMath::mat4f m_projection;
};