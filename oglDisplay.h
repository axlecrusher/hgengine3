#pragma once

#include <glew.h>
#include <vertex.h>
#include <HgElement.h>

#include <HgTypes.h>

#include <HgVbo.h>

#define U_VIEW 4
#define U_PROJECTION 5
#define U_ROTATION		1
#define U_POSITION		3
#define U_CAMERA_ROT 6
#define U_CAMERA_POS 7

#define L_VERTEX	0
#define L_NORMAL	1
#define L_UV		2
#define L_COLOR		3

extern viewport view_port[];
extern HgCamera* _camera;
extern float* _projection;

typedef struct ogl_vbo {
	GLuint* id;
	uint8_t count;
} ogl_vbo;

typedef struct OGLRenderData {
	RenderData baseRender;

	struct HgVboMemory* hgVbo;
	uint32_t vbo_offset;
	uint16_t vertex_count;

	GLuint idx_id;
	uint32_t index_count;
	void* indices;
} OGLRenderData;

GLuint hgOglVbo(vertices v);

void ogl_destroy_renderData();
void setGlobalUniforms(const HgCamera* camera);
void setLocalUniforms(const quaternion* rotation, const point* position, float scale);
void hgViewport(uint8_t vp);

void setup_viewports(uint16_t width, uint16_t height);
void ogl_render_renderData(RenderData* rd);

GLuint new_index_buffer8(uint8_t* indices, uint32_t count);
GLuint new_index_buffer16(uint16_t* indices, uint32_t count);
void setBlendMode(BlendMode blendMode);
void* new_renderData_ogl();
