#include <oglDisplay.h>

#include <stdlib.h>
#include <HgElement.h>
#include <oglShaders.h>

#include <string.h>
#include <HgVbo.h>

viewport view_port[3];

static uint8_t _currenViewPort_idx =  0xFF;
static BlendMode _currentBlendMode = BLEND_INVALID;

HgCamera* _camera;
float* _projection;

char *UniformString[] = {
	"rotation",
	"translation",
	"view",
	"projection",
	"cam_rot",
	"cam_position",
	"origin",
	"diffuseTex",
	"specularTex",
	"normalTex",
	NULL
};

void hgViewport(uint8_t idx) {
	if (idx == _currenViewPort_idx) return;
	_currenViewPort_idx = idx;

	const viewport vp = view_port[idx];
	glViewport(vp.x, vp.y, vp.width, vp.height);
	glScissor(vp.x, vp.y, vp.width, vp.height);
	glEnable(GL_SCISSOR_TEST);
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
	else if (blendMode == BLEND_ALPHA) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

static void default_render(RenderData* rd) {
	OGLRenderData *d = (OGLRenderData*)rd;

	setBlendMode(rd->blendMode);
	d->hgVbo->use();
	d->indexVbo->use();
	d->indexVbo->draw(d->index_count, d->vbo_offset, d->index_offset);
}

OGLRenderData::OGLRenderData()
	:RenderData(),hgVbo(nullptr),
	indexVbo(nullptr), colorVbo(nullptr),
	vbo_offset(0), vertex_count(0), idx_id(0), index_count(0)
{
	memset(&indices, 0, sizeof(indices));
	renderFunction = default_render;
	memset(textureID, 0, sizeof(textureID));
	init();
}

OGLRenderData::~OGLRenderData() {
	destroy();
}

void OGLRenderData::init() {
	shader = HgShader::acquire("test_vertex.glsl", "test_frag.glsl");
}

void OGLRenderData::destroy() {
//	OGLRenderData* oglrd = (OGLRenderData*)rd;
	free_arbitrary(&indices);
//	if (idx_id>0) glDeleteBuffers(1, &idx_id);

	//FIXME: Do something to clean up hgVbo
	//hgvbo_remove(d->hgvbo, d->vbo_offset, d->vertex_count)

	if (idx_id > 0) {
		glDeleteBuffers(1, &idx_id);
		idx_id = 0;
	}

	RenderData::destroy();
}
/*
static GLint colorDepth(HgTexture::channels c) {
	switch (c) {
	case HgTexture::channels::GRAY:
		return 0;
	case HgTexture::channels::GRAY_ALPHA:
		return 0;
	case HgTexture::channels::RGB:
		return GL_RGB8;
	case HgTexture::channels::RGBA:
		return GL_RGBA8;
	}
	return 0; //crash
}
*/
uint32_t ogl_updateTextureData(HgTexture* tex) {
	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLint internal, format;

	switch (tex->m_format) {
	case HgTexture::format::GRAY:
		internal = 0;
		format = 0;
		break;
	case HgTexture::format::GRAY_ALPHA:
		internal = 0;
		format = 0;
		break;
	case HgTexture::format::RGB:
		//			internal = GL_RGB8;
		internal = GL_RGB;
		format = GL_RGB;
		break;
	case HgTexture::format::RGBA:
		//			internal = GL_RGBA8;
		internal = GL_RGBA;
		format = GL_RGBA;
		break;
	case HgTexture::format::DXT1:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case HgTexture::format::DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case HgTexture::format::DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	}

	if (tex->m_format < 0xFF) {
		glTexImage2D(GL_TEXTURE_2D, 0, internal, tex->m_width, tex->m_height, 0, format, GL_UNSIGNED_BYTE, tex->data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
		unsigned int offset = 0;

		uint32_t width = tex->m_width;
		uint32_t height = tex->m_height;

		for (uint32_t level = 0; level < tex->m_mipMapCount && (width || height); ++level)
		{
			uint32_t size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
			glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, tex->data + offset);

			offset += size;
			width /= 2;
			height /= 2;
		}

	}

	return id;
}

void OGLRenderData::clearTextureIDs() {
	memset(textureID, 0, sizeof(textureID));
}

void OGLRenderData::setTexture(const HgTexture* t) {
	HgTexture::TextureType type = t->getType();
	textureID[type] = t->getGPUId();
}
