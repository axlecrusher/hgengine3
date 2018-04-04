#pragma once

#include <HgTypes.h>
#include <oglDisplay.h>
#include <assert.h>

#include <oglDisplay.h>

/*	Interleaved vertex layout because it is faster to resize when adding
	more mesh data. New mesh data can just be appened to the end. If it
	were stored VVVNNNCCC, it would be much more difficult to add new
	mesh data as all other mesh data would need to be shifted around. I'm
	unsure of the impact that interleaved data has on rendering performance.
	It doesn't seem any slower than separate VBO buffers for each type.

	We do gain performance from calling glVertexAttribPointer, glBindBuffer
	and glBindVertexArray less.
*/

/*	NOTE: We should try to keep 4 by alignment.
	Opengl wiki says "The alignment of any attribute's data should be no
	less than 4 bytes. So if you have a vec3 of GLushorts, you can't use
	that 4th component for a new attribute (such as a vec2 of GLbytes).
	If you want to pack something into that instead of having useless
	padding, you need to make it a vec4 of GLushorts."
*/

static void* _currentVbo;

enum VBO_TYPE : uint8_t {
	VBO_TYPE_INVALID = 0,
	VBO_VC,
	VBO_VN,
	VBO_VNU,
	VBO_VNUT,
	VBO_INDEX8,
	VBO_INDEX16,
	VBO_COLOR8
};


//Vertex,Color
typedef struct vbo_layout_vc {
	vertex v;
	color c;
} vbo_layout_vc;


//Vertex,Normal
typedef struct vbo_layout_vn {
	vertex v;
	normal n;
} vbo_layout_vn;

//Vertex,Normal,UV
typedef struct vbo_layout_vnu {
	vertex v;
	normal n;
	uv_coord uv;
} vbo_layout_vnu;

typedef struct vbo_layout_vnut {
	vertex v;
	normal n;
	tangent tan;
	uv_coord uv;
} vbo_layout_vnut;

class HgVboBase {
public:

	//	Copy data straight into VBO. Ensure that data, is using the correct vbo layout for your data
	virtual uint32_t add_data(void* data, uint16_t vertex_count) = 0;

	//	Clear data from RAM. Will not upload new data to GPU until new data is added.
	virtual void clear() = 0;

	virtual void destroy() = 0;

	virtual void use() = 0;

	virtual void draw(uint32_t offset) = 0;

	virtual VBO_TYPE VboType() = 0;
};

struct OGLVboId {
	GLuint vbo_id;
	GLuint vao_id;
};

template<typename T>
class HgVboMemory : public HgVboBase{
public:
	HgVboMemory();
	virtual ~HgVboMemory();

	uint32_t add_data(void* data, uint16_t vertex_count);
	void clear();
	void destroy();

	void use();

	virtual void draw(uint32_t offset) { ::draw_vbo<T>(this, offset); }
	T* getBuffer() { return buffer;  }
	void setNeedsUpdate(bool x) { needsUpdate = x; }

	inline uint32_t getCount() const { return count; }

	virtual VBO_TYPE VboType() { return HgVboMemory<T>::Type(); }
private:
	static constexpr VBO_TYPE Type() {
		//looks stupid but is compile time evaluated
		if (std::is_same<T, vbo_layout_vc>::value) { return VBO_VC; }
		if (std::is_same<T, vbo_layout_vn>::value) { return VBO_VN; }
		if (std::is_same<T, vbo_layout_vnu>::value) { return VBO_VNU; }
		if (std::is_same<T, vbo_layout_vnut>::value) { return VBO_VNUT; }
		if (std::is_same<T, uint8_t>::value) { return VBO_INDEX8; }
		if (std::is_same<T, uint16_t>::value) { return VBO_INDEX16; }
		if (std::is_same<T, color>::value) { return VBO_COLOR8; }
		return VBO_TYPE_INVALID;
	}

	static constexpr uint8_t Stride() { return sizeof(T); }

	T* buffer;

	uint32_t count;
	bool needsUpdate;

	union {
		struct OGLVboId ogl;
	} handle;

	T* HgVboMemory::resize(uint32_t count);
	void sendToGPU();
};



template<typename T>
HgVboMemory<T>::HgVboMemory()
	:buffer(nullptr), count(0), needsUpdate(0)
{
	memset(&handle, 0, sizeof(handle));
	static_assert(Type() != VBO_TYPE_INVALID, "Invalid VBO Type");
}

template<typename T>
HgVboMemory<T>::~HgVboMemory() {
	if (buffer != nullptr) free(buffer);
	buffer = nullptr;

	destroy();
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
	if (handle.ogl.vbo_id>0) glDeleteBuffers(1, &handle.ogl.vbo_id);
	if (handle.ogl.vao_id>0) glDeleteBuffers(1, &handle.ogl.vao_id);
	handle.ogl.vao_id = handle.ogl.vbo_id = 0;
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
void HgVboMemory<T>::sendToGPU() {
	if (handle.ogl.vbo_id == 0) glGenBuffers(1, &handle.ogl.vbo_id);
	if (handle.ogl.vao_id == 0) glGenVertexArrays(1, &handle.ogl.vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, handle.ogl.vbo_id);
	glBufferData(GL_ARRAY_BUFFER, count * Stride(), buffer, GL_STATIC_DRAW);

	glBindVertexArray(handle.ogl.vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, handle.ogl.vbo_id);
	//minimize calls to glVertexAttribPointer, use same format for all meshes in a VBO

	glVertexAttribPointer(L_VERTEX, 3, GL_FLOAT, GL_FALSE, Stride(), NULL);
	glEnableVertexAttribArray(L_VERTEX); //enable access to attribute

	//Type() is a constexpr so maybe the compiler will optimize away these if statements... (test it sometime)
	if ((Type() == VBO_VC) || (Type() == VBO_COLOR8)) {
		glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, Stride(), (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_COLOR);
	}
	else if (Type() == VBO_VN) {
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, Stride(), (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_NORMAL);
	}
	else if (Type() == VBO_VNU) {
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, Stride(), (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_NORMAL);
		glVertexAttribPointer(L_UV, 2, GL_UNSIGNED_SHORT, GL_TRUE, Stride(), (void*)(sizeof(vertex) + sizeof(normal)));
		glEnableVertexAttribArray(L_UV);
	}
	else if (Type() == VBO_VNUT) {
		int offset = sizeof(vertex);
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, Stride(), (void*)offset);
		glEnableVertexAttribArray(L_NORMAL);
		offset += sizeof(normal);
		glVertexAttribPointer(L_TANGENT, 4, GL_FLOAT, GL_FALSE, Stride(), (void*)offset);
		glEnableVertexAttribArray(L_TANGENT);
		offset += sizeof(tangent); //tangent normals
		glVertexAttribPointer(L_UV, 2, GL_UNSIGNED_SHORT, GL_TRUE, Stride(), (void*)offset);
		glEnableVertexAttribArray(L_UV);
	}
	else {
		fprintf(stderr, "Unknown vbo type:%d\n", Type());
		assert(!"Unknown vbo type");
	}

	needsUpdate = false;
}

template<typename T>
inline void draw_vbo(HgVboMemory<T>* vbo, uint32_t offset) {}

//NOTE: THESE ARE INLINE. THEY NEED TO BE IN THE HEADER, NOT CPP
template<>
inline void draw_vbo(HgVboMemory<uint8_t>* vbo, uint32_t offset) {
	glDrawElementsBaseVertex(GL_TRIANGLES, vbo->getCount(), GL_UNSIGNED_BYTE, 0, offset);
}

template<>
inline void draw_vbo(HgVboMemory<uint16_t>* vbo, uint32_t offset) {
	glDrawElementsBaseVertex(GL_TRIANGLES, vbo->getCount(), GL_UNSIGNED_SHORT, 0, offset);
}


extern HgVboMemory<vbo_layout_vc> staticVbo;
extern HgVboMemory<vbo_layout_vnu> staticVboVNU;
extern HgVboMemory<vbo_layout_vnut> staticVboVNUT;