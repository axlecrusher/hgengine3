#include <WindowRenderTarget.h>
#include <HgUtils.h>
#include <HgCamera.h>
#include <MercuryWindow.h>
#include <RenderBackend.h>

WindowRenderTarget::WindowRenderTarget()
{

}

bool WindowRenderTarget::Init()
{
	ENGINE::StartWindowSystem(MercuryWindow::Dimensions(1280, 720));

	auto window = MercuryWindow::GetCurrentWindow();

	m_windowViewport.width = window->CurrentWidth();
	m_windowViewport.height = window->CurrentHeight();

	return true;
}

HgMath::mat4f WindowRenderTarget::getPerspectiveMatrix() const
{
	HgMath::mat4f projectionMatrix;

	float projection[16];

	const double width = m_windowViewport.width;
	const double height = m_windowViewport.height;
	const double aspect = width / height;

	//Perspective2(60, aspect, 0.1f, 100.0f, projection);
	Projection_RH_InfZ_RevDepth(60, aspect, 0.1f, projection);
	projectionMatrix.load(projection);

	return projectionMatrix;
}

HgMath::mat4f WindowRenderTarget::getOrthoMatrix() const
{
	//return getPerspectiveMatrix();
	return vectorial::transpose(HgMath::mat4f::ortho(-1, 1, -1, 1, -1, 1));
}

void WindowRenderTarget::Render(const RenderParamsList& l)
{
	//If the display resolution changed we need to know
	auto window = MercuryWindow::GetCurrentWindow();
	m_windowViewport.width = window->CurrentWidth();
	m_windowViewport.height = window->CurrentHeight();

	RENDERER()->Clear();
	RENDERER()->BeginFrame();

	for (const RenderParams& i : l)
	{
		const auto hdmCamMatrix = i.camera->toViewMatrix();
		const auto projection = i.projection->getProjectionMatrix(*this);
		Renderer::Render(m_windowViewport, hdmCamMatrix, projection, i.queue);
	}

	//Renderer::Render(m_windowViewport, camera->toViewMatrix(), projection, queue);
}