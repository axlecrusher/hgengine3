#pragma once

#include <glew.h>
#include <vertex.h>
#include <HgElement.h>

#include <HgTypes.h>

#define U_VIEW 4
#define U_PROJECTION 5
#define U_ROTATION		1
#define U_POSITION		3
#define U_CAMERA_ROT 6
#define U_CAMERA_POS 7

#define L_VERTEX	0
#define L_COLOR		1

extern viewport view_port[];
extern HgCamera* _camera;
extern float* _projection;

typedef struct ogl_vbo {
	GLuint* id;
	uint8_t count;
} ogl_vbo;

typedef struct OGLRenderData {
	RenderData baseRender;
	GLuint vao;
	ogl_vbo vbo;
} OGLRenderData;

GLuint hgOglVbo(vertices v);

void ogl_destroy_renderData();
void setGlobalUniforms(const HgCamera* camera);
void setLocalUniforms(const quaternion* rotation, const point* position, float scale);
void hgViewport(uint8_t vp);

void setup_viewports(uint16_t width, uint16_t height);