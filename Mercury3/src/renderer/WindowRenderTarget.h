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
	virtual void Render(const RenderParamsList& l);

	virtual HgMath::mat4f getPerspectiveMatrix() const;
	virtual HgMath::mat4f getOrthoMatrix() const;

	virtual uint32_t getWidth() const { return m_windowViewport.width; };
	virtual uint32_t getHeight() const { return m_windowViewport.height; };

	virtual void BindFramebuffer() override;

private:
	//HgMath::mat4f m_projection;
	Viewport m_windowViewport;
};