#pragma once

#include <stdlib.h>
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

typedef struct ArbitraryData {
	void* data;
	uint8_t owns_ptr;
} ArbitraryData;

inline void free_arbitrary(ArbitraryData* x) {
	if (x->owns_ptr == 0 || x->data == NULL) return;
	free(x->data);
	x->data = NULL;
	x->owns_ptr = 0;
}

typedef struct OGLRenderData {
	RenderData baseRender;

	struct HgVboMemory* hgVbo;
	struct HgVboMemory* indexVbo;
	struct HgVboMemory* colorVbo;

	uint32_t vbo_offset;
	uint16_t vertex_count;

	GLuint idx_id;
	uint32_t index_count;

	ArbitraryData indices;
} OGLRenderData;

GLuint hgOglVbo(vertices v);

//void destroy_render_data_ogl(struct RenderData* render_data);
//void ogl_destroy_renderData();
void setGlobalUniforms(const HgCamera* camera);
void setLocalUniforms(const quaternion* rotation, const point* position, float scale);
void hgViewport(uint8_t vp);

void setup_viewports(uint16_t width, uint16_t height);
void ogl_render_renderData(RenderData* rd);

GLuint new_index_buffer8(uint8_t* indices, uint32_t count);
GLuint new_index_buffer16(uint16_t* indices, uint32_t count);
void setBlendMode(BlendMode blendMode);
void* new_renderData_ogl();

void draw_index_vbo(struct HgVboMemory* vbo, uint32_t offset);

inline OGLRenderData* getOglRenderData(HgElement* e) { return (OGLRenderData*)e->m_renderData; }