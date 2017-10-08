#include <HgVbo.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <string.h>

HgVboMemory<vbo_layout_vc> staticVbo;
HgVboMemory<vbo_layout_vnu> staticVboVNU;

//static void* _currentVbo;

static uint16_t struct_size(VBO_TYPE type) {
	if (type == VBO_VC) return sizeof(vbo_layout_vc);
	if (type == VBO_VNU) return sizeof(vbo_layout_vnu);
	if (type == VBO_INDEX8) return sizeof(uint8_t);
	if (type == VBO_INDEX16) return sizeof(uint16_t);
	if (type == VBO_COLOR8) return sizeof(color);

	fprintf(stderr, "Unknown vbo type:%d\n", type);
	assert(!"Unknown vbo type");
	return 0;
}

template<typename T>
void HgVboMemory<T>::use() {
	if ((_currentVbo == this) && (needsUpdate == false)) return;

	_currentVbo = this;

	if (needsUpdate) {
		hgvbo_sendogl();
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id); //is this needed or does the vao_id do this for us?
	glBindVertexArray(vao_id);
}


//8 bit index
template<>
void HgVboMemory<uint8_t>::use() {
	if (vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		vbo_id = buf_id;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_id);

	if (needsUpdate) {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * stride, buffer, GL_STATIC_DRAW);
		needsUpdate = false;
	}
}

//16 bit index
template<>
void HgVboMemory<uint16_t>::use() {
	if (vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		vbo_id = buf_id;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_id);

	if (needsUpdate) {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * stride, buffer, GL_STATIC_DRAW);
		needsUpdate = false;
	}
}

template<>
void HgVboMemory<color>::use() {
	if (vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		vbo_id = buf_id;
		/*
		glGenVertexArrays(1, &vbo->vao_id);
		glBindVertexArray(vbo->vao_id);
		glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(L_COLOR);
		*/
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

	if (needsUpdate) {
		color* c = buffer;
		glBufferData(GL_ARRAY_BUFFER, count * stride, buffer, GL_STATIC_DRAW);
		needsUpdate = false;
	}

	glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
	glEnableVertexAttribArray(L_COLOR);
}