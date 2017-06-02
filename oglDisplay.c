#include <oglDisplay.h>

#include <stdlib.h>

viewport view_port[3];

static uint8_t _currenViewPort_idx =  0xFF;

HgCamera* _camera;
float* _projection;

void hgViewport(uint8_t idx) {
	if (idx == _currenViewPort_idx) return;
	_currenViewPort_idx = idx;

	const viewport vp = view_port[idx];
	glViewport(vp.x, vp.y, vp.width, vp.height);
	glScissor(vp.x, vp.y, vp.width, vp.height);
	glEnable(GL_SCISSOR_TEST);
}

void setGlobalUniforms() {
	glUniformMatrix4fv(U_PROJECTION, 1, GL_TRUE, _projection);
	glUniform4f(U_CAMERA_ROT, _camera->rotation.x, _camera->rotation.y, _camera->rotation.z, _camera->rotation.w);
	glUniform3f(U_CAMERA_POS, _camera->position.components.x, _camera->position.components.y, _camera->position.components.z);
}

void setLocalUniforms(quaternion* rotation, point* position, float scale) {
	glUniform4f(U_ROTATION, rotation->x, rotation->y, rotation->z, rotation->w);
	glUniform4f(U_POSITION, position->components.x, position->components.y, position->components.z, scale);
}

GLuint hgOglVbo(vertices v) {
	GLuint vbo = 0;
	GLuint vao = 0;

//	glGenVertexArrays(1, &vao);
//	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, v.size * sizeof(*(v.points.v)), v.points.array, GL_STATIC_DRAW);
	return vbo;
}

void OGLRenderData_destroy(OGLRenderData* d) {
	if (d->vbo.id != NULL) {
		glDeleteBuffers(d->vbo.count, d->vbo.id);
		d->vbo.count = 0;
		free(d->vbo.id);
		d->vbo.id = NULL;
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
	view_port[i].width = width * 0.5f;
	view_port[i].height = height;
	++i;

	view_port[i].x = width * 0.5f;
	view_port[i].y = 0;
	view_port[i].width = width * 0.5f;
	view_port[i].height = height;
}