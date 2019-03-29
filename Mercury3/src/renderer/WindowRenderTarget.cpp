#include <WindowRenderTarget.h>
#include <HgUtils.h>
#include <HgCamera.h>
#include <MercuryWindow.h>
#include <RenderBackend.h>

WindowRenderTarget::WindowRenderTarget()
{

}

void WindowRenderTarget::Init()
{
	ENGINE::StartWindowSystem(1920, 1080);

	auto window = MercuryWindow::GetCurrentWindow();

	int width = window->CurrentWidth();
	int height = window->CurrentHeight();

	float projection[16];

	RENDERER()->setup_viewports(width, height);
	double renderWidth = width;
	double renderHeight = height;
	double aspect = renderWidth / renderHeight;
	Perspective2(60, aspect, 0.1f, 100.0f, projection);
	m_projection.load(projection);
	//auto tmp = HgMath::mat4f::perspective(60*DEG_RAD, aspect, 0.1f, 100.0f);
	//tmp.store(projection);
}

void WindowRenderTarget::Render(HgCamera* camera, RenderQueue* queue)
{
	Renderer::Render(0, camera, m_projection, queue);
}