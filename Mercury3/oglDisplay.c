#include <oglDisplay.h>

#include <stdlib.h>
#include <HgElement.h>

viewport view_port[3];

static uint8_t _currenViewPort_idx =  0xFF;
static uint8_t _currentBlendMode = 0xFF;

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

void setGlobalUniforms(const HgCamera* c) {
	glUniformMatrix4fv(U_PROJECTION, 1, GL_TRUE, _projection);
	glUniform4f(U_CAMERA_ROT, c->rotation.x, c->rotation.y, c->rotation.z, c->rotation.w);
	glUniform3f(U_CAMERA_POS, c->position.components.x, c->position.components.y, c->position.components.z);
}

void setLocalUniforms(const quaternion* rotation, const point* position, float scale) {
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
	//FIXME: Do something to clean up hgVbo
	//hgvbo_remove(d->hgvbo, d->vbo_offset, d->vertex_count)

	if (d->idx_id > 0) {
		glDeleteBuffers(1, &d->idx_id);
		d->idx_id = 0;
	}
}

void setup_viewports(uint16_t width, uint16_t height) {
	uint8_t i = 0;

	view_port[i].x = view_port[i].y = 0;
	view_port[i].width = width;
	view_port[i].height = height;
	++i;

	view_port[i].x = view_port[i].y = 0;
	view_port[i].width = width/2;
	view_port[i].height = height;
	++i;

	view_port[i].x = width/2;
	view_port[i].y = 0;
	view_port[i].width = width/2;
	view_port[i].height = height;
}

void ogl_render_renderData(RenderData* rd) {
	OGLRenderData *d = (OGLRenderData*)rd;
	if (d->idx_id == 0) {
		d->idx_id = new_index_buffer8(d->indices.data, d->index_count);
		free_arbitrary(&d->indices);
	}

	setBlendMode(rd->blendMode);
	hgvbo_use(d->hgVbo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d->idx_id);
	glDrawElementsBaseVertex(GL_TRIANGLES, d->index_count, GL_UNSIGNED_BYTE, 0, d->vbo_offset);
}

GLuint new_index_buffer8(uint8_t* indices, uint32_t count) {
	GLuint buf_id;
	glGenBuffers(1, &buf_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(*indices), indices, GL_STATIC_DRAW);

	return buf_id;
}

GLuint new_index_buffer16(uint16_t* indices, uint32_t count) {
	GLuint buf_id;
	glGenBuffers(1, &buf_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(*indices), indices, GL_STATIC_DRAW);

	return buf_id;
}

void setBlendMode(BlendMode blendMode) {
	if (_currentBlendMode == blendMode) return;
	_currentBlendMode = blendMode;

	if (blendMode == BLEND_NORMAL) {
//		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
//		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else if (blendMode == BLEND_ADDITIVE) {
//		glDepthMask(GL_FALSE);
		glEnable (GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
}

static void destroy_render_data_ogl(struct RenderData* rd) {
	OGLRenderData* oglrd = (OGLRenderData*)rd;
	free_arbitrary(&oglrd->indices);
	if (oglrd->idx_id>0) glDeleteBuffers(1, &oglrd->idx_id);
	if (rd->shader) HGShader_release(rd->shader);
}

void* new_renderData_ogl() {
	OGLRenderData* rd = calloc(1, sizeof(*rd));
	rd->baseRender.renderFunc = ogl_render_renderData;
	rd->baseRender.blendMode = BLEND_NORMAL;
	rd->baseRender.shader = HGShader_acquire("test_vertex.glsl", "test_frag.glsl");
	rd->baseRender.destroy = destroy_render_data_ogl;

	return rd;
}

void draw_index_vbo(HgVboMemory* vbo, uint32_t offset) {
	if (vbo->type == VBO_INDEX8 || vbo->type == VBO_INDEX16) {
		uint32_t glType = GL_UNSIGNED_BYTE;
		if (vbo->type == VBO_INDEX16) glType = GL_UNSIGNED_SHORT;
		glDrawElementsBaseVertex(GL_TRIANGLES, vbo->count, glType, 0, offset);
	}
}