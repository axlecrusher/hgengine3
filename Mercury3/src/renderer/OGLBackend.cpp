#include <OGLBackend.h>

//#define GL_GLEXT_PROTOTYPES
//#include <glcorearb.h>
//#include <glext.h>
#include <glew.h>
#include <stdio.h>

#include <oglShaders.h>
#include <oglDisplay.h>

#include <HgVbo.h>

//OGLBackend oglRenderer;

static void GLAPIENTRY ogl_error_clbk(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	//fprintf(stderr, "%s\n", message);
}

RenderBackend* OGLBackend::Create() {
	static OGLBackend openglRenderer;

	HgShader::Create = HgOglShader::Create;
	RenderData::Create = new_renderData_ogl;
	HgTexture::updateTextureFunc = ogl_updateTextureData;

	return &openglRenderer;
}

void OGLBackend::Init() {

	GLenum err = glewInit();

	printf("%s\n", glGetString(GL_VERSION));
	printf("%s\n", glGetString(GL_VENDOR));

	GLint d;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &d);
	printf("GL_MAX_VERTEX_ATTRIBS %d\n", d);

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &d);
	printf("GL_MAX_VERTEX_UNIFORM_COMPONENTS %d\n", d);

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &d);
	printf("GL_MAX_VERTEX_UNIFORM_VECTORS %d\n", d);

	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &d);
	printf("GL_MAX_VERTEX_UNIFORM_BLOCKS %d\n", d);

	glDebugMessageCallback(ogl_error_clbk, NULL);
	glEnable(GL_DEBUG_OUTPUT);

	//	glEnable(GL_MULTISAMPLE);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void OGLBackend::Clear() {
	//Write masking state glColorMask, glStencilMask and glDepthMask can affect Framebuffer Clearing functionality.
	glDepthMask(GL_TRUE); //glClear requires depth mask
	glDisable(GL_SCISSOR_TEST);
	glClearColor(.1f, .1f, .1f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OGLBackend::BeginFrame() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void OGLBackend::EndFrame() {

}

void OGLBackend::Viewport(uint8_t idx) {
	if (idx == _currenViewPort_idx) return;
	_currenViewPort_idx = idx;

	const viewport vp = view_port[idx];
	glViewport(vp.x, vp.y, vp.width, vp.height);
	glScissor(vp.x, vp.y, vp.width, vp.height);
	glEnable(GL_SCISSOR_TEST);
}

void OGLBackend::setRenderAttributes(BlendMode blendMode, RenderFlags flags) {
	//if (_currentBlendMode == blendMode) return;
	//_currentBlendMode = blendMode;

	switch (blendMode) {
	case BLEND_NORMAL:
		//glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		//		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BLEND_ADDITIVE:
		//		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case BLEND_ALPHA:
		//		glDepthMask(GL_TRUE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	}

	if ((flags & FACE_CULLING) > 0) {
		glEnable(GL_CULL_FACE);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	if ((flags & DEPTH_WRITE) > 0) {
		glDepthMask(GL_TRUE);
	}
	else {
		glDepthMask(GL_FALSE);
	}
}