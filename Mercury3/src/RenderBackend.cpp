#include <RenderBackend.h>
#include <OGLBackend.h>

//replace with some kind of configure based factory thing
RenderBackend* RENDERER = &oglRenderer;

namespace Renderer {
	void InitOpenGL() {
		OGLBackend::Init();
	}
}