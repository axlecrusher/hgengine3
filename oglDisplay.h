#pragma once

#include <glew.h>
#include <vertex.h>
#include <HgElement.h>

#include <HgTypes.h>

#define U_VIEW 4
#define U_PROJECTION 5
#define U_CAMERA_ROT 6
#define U_CAMERA_POS 7

extern viewport view_port[];
extern HgCamera* _camera;
extern float* _projection;

typedef struct OGLRenderData {
	RenderData baseRender;
	GLuint vao;
	GLuint* vbo;
	uint8_t vbo_size;

	GLuint shader_program;
} OGLRenderData;

GLuint hgOglVbo(vertices v);

void ogl_destroy_renderData();
void setGlobalUniforms();
void hgViewport(uint8_t vp);

void setup_viewports(uint16_t width, uint16_t height);