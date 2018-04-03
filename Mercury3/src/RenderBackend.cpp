#include <RenderBackend.h>
#include <OGLBackend.h>

RenderBackend* RENDERER;

namespace Renderer {
	void InitOpenGL() {
		OGLBackend::Init();
	}
}