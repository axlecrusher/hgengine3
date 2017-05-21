#include <oglDisplay.h>

#include <stdlib.h>

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

