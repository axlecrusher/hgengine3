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

//OGLRenderData::OGLRenderData()
//	:RenderData()
//{
//	memset(textureID, 0, sizeof(textureID));
//	init();
//}

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
	GLuint id = tex->getGPUId();

	if (tex->getGPUId() == 0)
	{
		glGenTextures(1, &id);
	}

	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	const auto properties = tex->getProperties();

	GLint min_filter = GL_LINEAR;
	if (properties.mipMapCount > 1) {
		min_filter = GL_LINEAR_MIPMAP_LINEAR;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLint internal, format;

	switch (properties.format) {
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
	case HgTexture::format::BC5U_ATI:
	case HgTexture::format::BC5U:
		format = GL_COMPRESSED_RG_RGTC2;
		break;
	case HgTexture::format::BC5S:
		format = GL_COMPRESSED_SIGNED_RG_RGTC2;
		break;
	}

	auto ltd = tex->getLoadedTextureData();

	if (properties.format < 0xFF) {
		glTexImage2D(GL_TEXTURE_2D, 0, internal, properties.width, properties.height, 0, format, GL_UNSIGNED_BYTE, ltd->getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
		unsigned int offset = 0;

		uint32_t width = properties.width;
		uint32_t height = properties.height;

		for (uint32_t level = 0; level < properties.mipMapCount && (width || height); ++level)
		{
			uint32_t size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
			glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, ltd->getData() + offset);

			offset += size;
			width /= 2;
			height /= 2;
		}

	}

	return id;
}

//void OGLRenderData::clearTextureIDs() {
//	memset(textureID, 0, sizeof(textureID));
//}
//
//void OGLRenderData::setTexture(const HgTexture* t) {
//	HgTexture::TextureType type = t->getType();
//	textureID[type] = t->getGPUId();
//}
