#include <HgVbo.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <string.h>

HgVboMemory<vbo_layout_vc> staticVbo;
HgVboMemory<vbo_layout_vnu> staticVboVNU;

static void* _currentVbo;

static uint16_t struct_size(VBO_TYPE type) {
	if (type == VBO_VC) return sizeof(vbo_layout_vc);
	if (type == VBO_VNU) return sizeof(vbo_layout_vnu);
	if (type == VBO_INDEX8) return sizeof(uint8_t);
	if (type == VBO_INDEX16) return sizeof(uint16_t);
	if (type == VBO_COLOR8) return sizeof(color);

	fprintf(stderr, "Unknown vbo type:%d\n", type);
	assert(!"Unknown vbo type");
	return 0;
}

static VBO_TYPE getVboType(const vbo_layout_vc& x) { return VBO_VC; }
static VBO_TYPE getVboType(const vbo_layout_vnu& x) { return VBO_VNU; }
static VBO_TYPE getVboType(const uint8_t& x) { return VBO_INDEX8; }
static VBO_TYPE getVboType(const uint16_t& x) { return VBO_INDEX16; }
static VBO_TYPE getVboType(const color& x) { return VBO_VC; }

template<typename T>
HgVboMemory<T>::HgVboMemory()
	:buffer(nullptr)
{

}

template<typename T>
HgVboMemory<T>::~HgVboMemory() {
	if (buffer != nullptr) free(buffer);
	buffer = nullptr;

	destroy();
}

template<typename T>
void HgVboMemory<T>::init() {
	type = getVboType(*buffer);
	stride = sizeof(T);
}

template<typename T>
T* HgVboMemory<T>::resize(uint32_t count) {
	T* buf = (T*)realloc(buffer, count * sizeof(*buf));
	assert(buf != NULL);
	buffer = buf;

	return buf;
}

template<typename T>
void HgVboMemory<T>::clear() {
	if (buffer != nullptr) free(buffer);
	buffer = NULL;
	count = 0;
}

template<typename T>
void HgVboMemory<T>::destroy() {
	if (vbo_id>0) glDeleteBuffers(1, &vbo_id);
	if (vao_id>0) glDeleteBuffers(1, &vao_id);
	vao_id = vbo_id = 0;
	clear();
}

template<typename T>
uint32_t HgVboMemory<T>::add_data(void* data, uint16_t vertex_count) {
	T* buf = buffer = resize(count + vertex_count);
	buf = buf + count;

	memcpy(buf, data, sizeof(*buf)*vertex_count);

	uint32_t offset = count;
	count += vertex_count;
	needsUpdate = true;

	return offset;
}

template<typename T>
void HgVboMemory<T>::hgvbo_sendogl() {
	if (vbo_id == 0) glGenBuffers(1, &vbo_id);
	if (vao_id == 0) glGenVertexArrays(1, &vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, count * stride, buffer, GL_STATIC_DRAW);

	glBindVertexArray(vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	//minimize calls to glVertexAttribPointer, use same format for all meshes in a VBO

	glVertexAttribPointer(L_VERTEX, 3, GL_FLOAT, GL_FALSE, stride, NULL);
	glEnableVertexAttribArray(L_VERTEX); //enable access to attribute

	if (type == VBO_VC) {
		glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_COLOR);
	}
	else if (type == VBO_VN) {
		glVertexAttribPointer(L_NORMAL, 4, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_NORMAL);
	}
	else if (type == VBO_VNU) {
		glVertexAttribPointer(L_NORMAL, 4, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_NORMAL);
		glVertexAttribPointer(L_UV, 2, GL_UNSIGNED_SHORT, GL_FALSE, stride, (void*)(sizeof(vertex)+sizeof(normal)));
		glEnableVertexAttribArray(L_UV);
	}
	else {
		fprintf(stderr, "Unknown vbo type:%d\n", type);
		assert(!"Unknown vbo type");
	}

	needsUpdate = false;
}

template<typename T>
void HgVboMemory<T>::use() {
	if ((_currentVbo == this) && (needsUpdate == false)) return;

	_currentVbo = this;

	if (needsUpdate) {
		hgvbo_sendogl();
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id); //is this needed or does the vao_id do this for us?
	glBindVertexArray(vao_id);
}

//8 bit index
template<>
void HgVboMemory<uint8_t>::use() {
	if (vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		vbo_id = buf_id;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_id);

	if (needsUpdate) {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * stride, buffer, GL_STATIC_DRAW);
		needsUpdate = false;
	}
}

template<>
void HgVboMemory<color>::use() {
	if (vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		vbo_id = buf_id;
/*
		glGenVertexArrays(1, &vbo->vao_id);
		glBindVertexArray(vbo->vao_id);
		glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(L_COLOR);
		*/
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

	if (needsUpdate) {
		color* c = buffer;
		glBufferData(GL_ARRAY_BUFFER, count * stride, buffer, GL_STATIC_DRAW);
		needsUpdate = false;
	}

	glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
	glEnableVertexAttribArray(L_COLOR);
}