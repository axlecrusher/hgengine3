#pragma once

#include <IRenderTarget.h>
#include <RenderBackend.h> //for  viewport
class StereoCrossEye : public IRenderTarget
{
public:
	StereoCrossEye();

	~StereoCrossEye()
	{}

	virtual bool Init();
	virtual void Render(HgCamera* camera, RenderQueue* queue);

private:
	HgMath::mat4f m_projection;

	Viewport m_leftEyeVp;
	Viewport m_rightEyeVp;
};