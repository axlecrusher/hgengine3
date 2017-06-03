#include <HgRenderQueue.h>

#include <oglDisplay.h>

int8_t draw_render_packet(const render_packet* p) {
	if (p->element == NULL) return 1;

	hgViewport(p->viewport_idx);

//	_camera = &p->camera;
//	glUniform4f(U_CAMERA_ROT, p->camera.rotation.x, p->camera.rotation.y, p->camera.rotation.z, p->camera.rotation.w);
//	glUniform3f(U_CAMERA_POS, p->camera.position.components.x, p->camera.position.components.y, p->camera.position.components.z);
	VCALL(p->element->m_renderData->shader, enable);

	//perspective and camera probably need to be rebound here as well. (if the shader program changed. uniforms are local to shader programs).
	//we could give each shader program a "needsGlobalUniforms" flag that is reset every frame, to check if uniforms need to be updated

	setGlobalUniforms(&p->camera);
	setLocalUniforms(&p->rotation, &p->position, p->scale);

	p->element->m_renderData->renderFunc(p->element);
	return 0;
}
