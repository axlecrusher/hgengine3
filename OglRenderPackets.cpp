#include <HgRenderQueue.h>

extern "C" {
#include <oglDisplay.h>
}

bool EndOfFrame::render() {
	return false;
};

bool RenderElement::render() {
	glUniform4f(U_CAMERA_ROT, camera.rotation.x, camera.rotation.y, camera.rotation.z, camera.rotation.w);
	glUniform3f(U_CAMERA_POS, camera.position.components.x, camera.position.components.y, camera.position.components.z);

	hgViewport(viewport_idx);

	element->m_renderData->renderFunc(element);
	return true;
}