#pragma once

#include <glew.h>
#include <vertex.h>
#include <HgElement.h>

typedef struct OGLRenderData {
	RenderData baseRender;
	GLuint vao;
	GLuint* vbo;
	uint8_t vbo_size;

	GLuint shader_program;
} OGLRenderData;

GLuint hgOglVbo(vertices v);

void ogl_destroy_renderData();
