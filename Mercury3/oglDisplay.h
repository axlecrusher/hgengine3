#pragma once

#include <stdlib.h>
#include <glew.h>
#include <vertex.h>
#include <HgElement.h>

#include <HgTypes.h>

//#include <HgVbo.h>
#include <memory>
#include <HgTexture.h>

enum UniformLocations {
	U_ROTATION=0,
	U_POSITION=1,
	U_VIEW=2,
	U_PROJECTION=3,
	U_CAMERA_ROT=4,
	U_CAMERA_POS=5,
	U_ORIGIN=6,
	U_DIFFUSE_TEXTURE=7,
	U_SPECULAR_TEXTURE = 8,
	U_NORMAL_TEXTURE = 9,
	U_UNIFORM_COUNT=10
};

extern char *UniformString[];

#define L_VERTEX	0
#define L_NORMAL	1
#define L_UV		2
#define L_COLOR		3
#define L_TANGENT	4

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

class OGLRenderData : public RenderData {
public:
	inline static OGLRenderData* Create() { return (OGLRenderData*)RenderData::Create(); }
	OGLRenderData();
	~OGLRenderData();
	virtual void init();
	virtual void destroy();
//	virtual void render();

	virtual void clearTextureIDs();
	virtual void setTexture(const HgTexture* t);

	class IHgVbo* hgVbo;

	//We need to be able to support multiple VBOs without hardcoding more here.
	class IHgVbo* indexVbo;
	class IHgVbo* colorVbo;

	uint32_t vbo_offset;
	uint16_t vertex_count;
//private:
	GLuint idx_id;
	uint32_t index_count;

//	std::shared_ptr<char*> indices;
	ArbitraryData indices; //can be in different formats, requiring different renderers, set renderFunction
//	indiceRenderFunc renderFunction;

	GLuint textureID[HgTexture::TEXTURE_TYPE_COUNT]; //zero ID to stop rendering
};

GLuint hgOglVbo(vertices v);

//void destroy_render_data_ogl(struct RenderData* render_data);
//void ogl_destroy_renderData();
void hgViewport(uint8_t vp);

void setup_viewports(uint16_t width, uint16_t height);
void ogl_render_renderData(RenderData* rd);

GLuint new_index_buffer8(uint8_t* indices, uint32_t count);
GLuint new_index_buffer16(uint16_t* indices, uint32_t count);
void setBlendMode(BlendMode blendMode);
inline RenderData* new_renderData_ogl() { return new OGLRenderData(); }

//void ogl_draw_vbo(HgVboMemory<uint8_t>* vbo, uint32_t offset);
//void ogl_draw_vbo(HgVboMemory<uint16_t>* vbo, uint32_t offset);

inline OGLRenderData* getOglRenderData(HgElement* e) { return (OGLRenderData*)e->renderData(); }

void Indice8Render(RenderData* rd);
void Indice16Render(RenderData* rd);

//uint32_t ogl_updateTextureData(uint16_t x, uint16_t y, HgTexture::channels c, unsigned char* data);
uint32_t ogl_updateTextureData(HgTexture* tex);