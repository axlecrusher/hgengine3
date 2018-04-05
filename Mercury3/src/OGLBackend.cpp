#include <OGLBackend.h>

//#define GL_GLEXT_PROTOTYPES
//#include <glcorearb.h>
//#include <glext.h>
#include <glew.h>
#include <stdio.h>

#include <oglShaders.h>
#include <oglDisplay.h>

#include <HgVbo.h>

OGLBackend oglRenderer;

static void GLAPIENTRY ogl_error_clbk(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "%s\n", message);
}

void OGLBackend::Init() {
	RENDERER = &oglRenderer;

	HgShader::Create = HgOglShader::Create;
	RenderData::Create = new_renderData_ogl;
	HgTexture::updateTextureFunc = ogl_updateTextureData;

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


template<typename T>
static void sendToGPU(HgVboMemory<T>* vbo) {
	auto& handle = vbo->getHandle();
	if (handle.ogl.vbo_id == 0) glGenBuffers(1, &handle.ogl.vbo_id);
	if (handle.ogl.vao_id == 0) glGenVertexArrays(1, &handle.ogl.vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, handle.ogl.vbo_id);
	glBufferData(GL_ARRAY_BUFFER, vbo->getCount() * vbo->Stride(), vbo->getBuffer(), GL_STATIC_DRAW);

	glBindVertexArray(handle.ogl.vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, handle.ogl.vbo_id);
	//minimize calls to glVertexAttribPointer, use same format for all meshes in a VBO

	glVertexAttribPointer(L_VERTEX, 3, GL_FLOAT, GL_FALSE, vbo->Stride(), NULL);
	glEnableVertexAttribArray(L_VERTEX); //enable access to attribute

	constexpr VBO_TYPE vbo_type = vbo->Type();

	//Compiler is not optimizing out the switch even though it could
	//maybe write template functions for each part
	switch (vbo_type) {
	case VBO_VC:
	case VBO_COLOR8:
		glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, vbo->Stride(), (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_COLOR);
		break;
	case VBO_VN:
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, vbo->Stride(), (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_NORMAL);
		break;
	case VBO_VNU:
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, vbo->Stride(), (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_NORMAL);
		glVertexAttribPointer(L_UV, 2, GL_UNSIGNED_SHORT, GL_TRUE, vbo->Stride(), (void*)(sizeof(vertex) + sizeof(normal)));
		glEnableVertexAttribArray(L_UV);
		break;
	case VBO_VNUT:
	{
		int offset = sizeof(vertex);
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, vbo->Stride(), (void*)offset);
		glEnableVertexAttribArray(L_NORMAL);
		offset += sizeof(normal);
		glVertexAttribPointer(L_TANGENT, 4, GL_FLOAT, GL_FALSE, vbo->Stride(), (void*)offset);
		glEnableVertexAttribArray(L_TANGENT);
		offset += sizeof(tangent); //tangent normals
		glVertexAttribPointer(L_UV, 2, GL_UNSIGNED_SHORT, GL_TRUE, vbo->Stride(), (void*)offset);
		glEnableVertexAttribArray(L_UV);
		break;
	}
	default:
		fprintf(stderr, "Unknown vbo type:%d\n", vbo_type);
		assert(!"Unknown vbo type");
	}
}

//8 bit index
template<>
static void sendToGPU(HgVboMemory<uint8_t>* vbo) {
	auto& handle = vbo->getHandle();
	if (handle.ogl.vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		handle.ogl.vbo_id = buf_id;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.ogl.vbo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vbo->getCount() * vbo->Stride(), vbo->getBuffer(), GL_STATIC_DRAW);
}

//16 bit index
template<>
static void sendToGPU(HgVboMemory<uint16_t>* vbo) {
	auto& handle = vbo->getHandle();
	if (handle.ogl.vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		handle.ogl.vbo_id = buf_id;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.ogl.vbo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vbo->getCount() * vbo->Stride(), vbo->getBuffer(), GL_STATIC_DRAW);
}

template<>
static void sendToGPU(HgVboMemory<color>* vbo) {
	auto& handle = vbo->getHandle();
	if (handle.ogl.vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		handle.ogl.vbo_id = buf_id;
		/*
		glGenVertexArrays(1, &vbo->vao_id);
		glBindVertexArray(vbo->vao_id);
		glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(L_COLOR);
		*/
	}

	glBindBuffer(GL_ARRAY_BUFFER, handle.ogl.vbo_id);
	glBufferData(GL_ARRAY_BUFFER, vbo->getCount() * vbo->Stride(), vbo->getBuffer(), GL_STATIC_DRAW);
}


void OGLBackend::sendToGPU(HgVboBase* vbo) {
	//I don't like this step.
	switch (vbo->VboType())
	{
	case VBO_TYPE_INVALID:
		break;
	case VBO_VC:
		::sendToGPU(static_cast<HgVboMemory<vbo_layout_vc>*>(vbo));
		break;
	case VBO_VN:
		::sendToGPU(static_cast<HgVboMemory<vbo_layout_vn>*>(vbo));
		break;
	case VBO_VNU:
		::sendToGPU(static_cast<HgVboMemory<vbo_layout_vnu>*>(vbo));
		break;
	case VBO_VNUT:
		::sendToGPU(static_cast<HgVboMemory<vbo_layout_vnut>*>(vbo));
		break;
	case VBO_INDEX8:
		::sendToGPU(static_cast<HgVboMemory<uint8_t>*>(vbo));
		break;
	case VBO_INDEX16:
		::sendToGPU(static_cast<HgVboMemory<uint16_t>*>(vbo));
		break;
	case VBO_COLOR8:
		::sendToGPU(static_cast<HgVboMemory<color>*>(vbo));
		break;
	default:
		break;
	}
}


template<typename T>
inline void bind(HgVboMemory<T>* vbo) {
	auto& handle = vbo->getHandle();
	glBindBuffer(GL_ARRAY_BUFFER, handle.ogl.vbo_id); //is this needed or does the vao_id do this for us?
	glBindVertexArray(handle.ogl.vao_id);
}


//8 bit index
template<>
inline void bind(HgVboMemory<uint8_t>* vbo) {
	auto& handle = vbo->getHandle();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.ogl.vbo_id);
}

//16 bit index
template<>
inline void bind(HgVboMemory<uint16_t>* vbo) {
	auto& handle = vbo->getHandle();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.ogl.vbo_id);
}

template<>
inline void bind(HgVboMemory<color>* vbo) {
	auto& handle = vbo->getHandle();
	glBindBuffer(GL_ARRAY_BUFFER, handle.ogl.vbo_id);
	glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
	glEnableVertexAttribArray(L_COLOR);
}

void OGLBackend::bind(HgVboBase* vbo) {
	//I don't like this step.
	switch (vbo->VboType())
	{
	case VBO_TYPE_INVALID:
		break;
	case VBO_VC:
		::bind(static_cast<HgVboMemory<vbo_layout_vc>*>(vbo));
		break;
	case VBO_VN:
		::bind(static_cast<HgVboMemory<vbo_layout_vn>*>(vbo));
		break;
	case VBO_VNU:
		::bind(static_cast<HgVboMemory<vbo_layout_vnu>*>(vbo));
		break;
	case VBO_VNUT:
		::bind(static_cast<HgVboMemory<vbo_layout_vnut>*>(vbo));
		break;
	case VBO_INDEX8:
		::bind(static_cast<HgVboMemory<uint8_t>*>(vbo));
		break;
	case VBO_INDEX16:
		::bind(static_cast<HgVboMemory<uint16_t>*>(vbo));
		break;
	case VBO_COLOR8:
		::bind(static_cast<HgVboMemory<color>*>(vbo));
		break;
	default:
		break;
	}
}