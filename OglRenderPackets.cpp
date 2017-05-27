#include <HgRenderQueue.h>

extern "C" {
#include <oglDisplay.h>
}

bool EndOfFrame::render() {
	return false;
};

bool RenderElement::render() {
	glUniform3f(7, cam_position[0], cam_position[1], cam_position[2]);
	hgViewport(vp);
	element->m_renderData->renderFunc(element);
	return true;
}