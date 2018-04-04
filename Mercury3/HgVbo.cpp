#include <HgVbo.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <string.h>
#include <RenderBackend.h>

HgVboMemory<vbo_layout_vc> staticVbo;
HgVboMemory<vbo_layout_vnu> staticVboVNU;
HgVboMemory<vbo_layout_vnut> staticVboVNUT;

//static void* _currentVbo;

template<typename T>
void HgVboMemory<T>::use() {
	if ((_currentVbo == this) && (needsUpdate == false)) return;

	_currentVbo = this;

	if (needsUpdate) {
		RENDERER->sendToGPU(this);
		needsUpdate = false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, handle.ogl.vbo_id); //is this needed or does the vao_id do this for us?
	glBindVertexArray(handle.ogl.vao_id);
}


//8 bit index
template<>
void HgVboMemory<uint8_t>::use() {
	if (needsUpdate) {
		RENDERER->sendToGPU(this);
		needsUpdate = false;
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.ogl.vbo_id);
}

//16 bit index
template<>
void HgVboMemory<uint16_t>::use() {
	if (needsUpdate) {
		RENDERER->sendToGPU(this);
		needsUpdate = false;
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.ogl.vbo_id);
}

template<>
void HgVboMemory<color>::use() {
	if (needsUpdate) {
		RENDERER->sendToGPU(this);
		needsUpdate = false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, handle.ogl.vbo_id);
	glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
	glEnableVertexAttribArray(L_COLOR);
}

