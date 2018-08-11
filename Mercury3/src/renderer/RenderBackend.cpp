#include <RenderBackend.h>
#include <OGLBackend.h>

#include <oglDisplay.h>

//replace with some kind of configure based factory thing
RenderBackend* RENDERER = &oglRenderer;

namespace Renderer {
	std::vector<HgElement*> opaqueElements;
	std::vector<HgElement*> transparentElements;

	void InitOpenGL() {
		OGLBackend::Init();
		//		setBlendMode(BLEND_NORMAL);
	}
}

void RenderBackend::setup_viewports(uint16_t width, uint16_t height) {
	uint8_t i = 0;

	view_port[i].x = view_port[i].y = 0;
	view_port[i].width = width;
	view_port[i].height = height;
	++i;

	view_port[i].x = view_port[i].y = 0;
	view_port[i].width = width / 2;
	view_port[i].height = height;
	++i;

	view_port[i].x = width / 2;
	view_port[i].y = 0;
	view_port[i].width = width / 2;
	view_port[i].height = height;
}

static void submit_for_render_serial(uint8_t viewport_idx, HgCamera* camera, HgElement* e) {
	RENDERER->Viewport(viewport_idx);

	HgShader* shader = e->renderData()->shader;

	//load texture data to GPU here. Can this be made to be done right after loading the image data, regardless of thread?
	if (e->flags.update_textures) {
		e->updateGpuTextures();
		e->flags.update_textures = false;
	}

	if (shader) {
		shader->enable();
		//perspective and camera probably need to be rebound here as well. (if the shader program changed. uniforms are local to shader programs).
		//we could give each shader program a "needsGlobalUniforms" flag that is reset every frame, to check if uniforms need to be updated
		shader->setGlobalUniforms(*camera);
		shader->setLocalUniforms(&e->rotation(), &e->position(), e->scale(), &e->origin(), e->renderData());
	}

	e->renderData()->render();
}

void Renderer::Render(uint8_t stereo_view, HgCamera* camera) {
	for (auto e : opaqueElements) {
		submit_for_render_serial(stereo_view, camera + 0, e);
	}

	for (auto e : transparentElements) {
		submit_for_render_serial(stereo_view, camera + 0, e);
	}
}