#include <StereoCrossEyeTarget.h>
#include <HgUtils.h>
#include <HgCamera.h>
#include <MercuryWindow.h>
#include <RenderBackend.h>
#include <Win32Window.h>

#define EYE_DISTANCE -0.07f

StereoCrossEye::StereoCrossEye()
{
}

bool StereoCrossEye::Init()
{
	ENGINE::StartWindowSystem(MercuryWindow::Dimensions(1280, 480));

	auto window = MercuryWindow::GetCurrentWindow();

	int width = window->CurrentWidth();
	int height = window->CurrentHeight();

	float projection[16];

	//left eye
	m_leftEyeVp.x = m_leftEyeVp.y = 0;
	m_leftEyeVp.width = width / 2;
	m_leftEyeVp.height = height;

	//right eye
	m_rightEyeVp.x = width / 2;
	m_rightEyeVp.y = 0;
	m_rightEyeVp.width = width / 2;
	m_rightEyeVp.height = height;

	double renderWidth = width / 2.0;
	double renderHeight = height;
	Perspective2(60, renderWidth / renderHeight, 0.1f, 100.0f, projection);
	m_projection.load(projection);

	return true;
}

void StereoCrossEye::Render(HgCamera* camera, RenderQueue* queue)
{
	HgCamera left_eye = *camera;
	left_eye.setWorldSpacePosition(ComputeStereoCameraPosition(left_eye, -EYE_DISTANCE * 0.5f));

	HgCamera right_eye = *camera;
	right_eye.setWorldSpacePosition(ComputeStereoCameraPosition(right_eye, EYE_DISTANCE * 0.5f));

	Renderer::Render(m_leftEyeVp, left_eye.toViewMatrix(), m_projection, queue); //eye 1
	Renderer::Render(m_rightEyeVp, right_eye.toViewMatrix(), m_projection, queue); //eye 2
}