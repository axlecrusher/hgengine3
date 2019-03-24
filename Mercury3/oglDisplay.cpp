#include <oglDisplay.h>

#include <stdlib.h>
#include <HgEntity.h>
#include <oglShaders.h>

#include <string.h>
#include <HgVbo.h>

static BlendMode _currentBlendMode = BLEND_INVALID;

//HgCamera* _camera;
//float* _projection;

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
	"bufferObject1",
	"modelMatrix",
	"matrices[0]",
	NULL
};

OGLRenderData::OGLRenderData()
	:RenderData()
{
	memset(textureID, 0, sizeof(textureID));
	init();
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
