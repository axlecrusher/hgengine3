#pragma once

#include <stdlib.h>
#include <glew.h>
#include <vertex.h>

#include <RenderData.h>
//#include <HgElement.h>

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
	U_BUFFER_OBJECT1 = 10,
	U_UNIFORM_COUNT=11
};

extern char *UniformString[];
extern float* _projection;

class OGLRenderData : public RenderData {
public:
	inline static OGLRenderData* Create() { return (OGLRenderData*)RenderData::Create(); }
	OGLRenderData();
//	virtual void render();

	virtual void clearTextureIDs();
	virtual void setTexture(const HgTexture* t);

	GLuint textureID[HgTexture::TEXTURE_TYPE_COUNT]; //zero ID to stop rendering
};

void default_render(RenderData* rd);

//void destroy_render_data_ogl(struct RenderData* render_data);
//void ogl_destroy_renderData();

//void ogl_render_renderData(RenderData* rd);


//void setBlendMode(BlendMode blendMode);
void setRenderAttributes(BlendMode blendMode, RenderFlags flags);

inline RenderData* new_renderData_ogl() { return new OGLRenderData(); }

//inline OGLRenderData* getOglRenderData(HgElement* e) { return (OGLRenderData*)e->renderData(); }

uint32_t ogl_updateTextureData(HgTexture* tex);