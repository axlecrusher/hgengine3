#include <oglDisplay.h>

#include <stdlib.h>

viewport view_port[3];

static uint8_t _currenViewPort_idx =  0xFF;

void hgViewport(uint8_t idx) {
	if (idx == _currenViewPort_idx) return;
	_currenViewPort_idx = idx;

	const viewport vp = view_port[idx];
	glViewport(vp.x, vp.y, vp.width, vp.height);
	glScissor(vp.x, vp.y, vp.width, vp.height);
	glEnable(GL_SCISSOR_TEST);
}


GLuint hgOglVbo(vertices v) {
	GLuint vbo = 0;
	GLuint vao = 0;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, v.f_size * sizeof(float), v.points.array, GL_STATIC_DRAW);
	return vbo;
}

void ogl_destroy_renderData(OGLRenderData* d) {
	if (d->vbo != NULL) {
		glDeleteBuffers(d->vbo_size, d->vbo);
		d->vbo_size = 0;
		free(d->vbo);
		d->vbo = NULL;
	}

	if (d->vao > 0) {
		glDeleteVertexArrays(1, &d->vao);
		d->vao = 0;
	}
}

void setup_viewports(uint16_t width, uint16_t height) {
	uint8_t i = 0;

	view_port[i].x = view_port[i].y = 0;
	view_port[i].width = width;
	view_port[i].height = height;
	++i;

	view_port[i].x = view_port[i].y = 0;
	view_port[i].width = width * 0.5;
	view_port[i].height = height;
	++i;

	view_port[i].x = width * 0.5;
	view_port[i].y = 0;
	view_port[i].width = width * 0.5;
	view_port[i].height = height;
}