#pragma once

#include <HgVbo.h>
#include <HgVboMemory.h>

#include <glew.h>

template<typename T>
class OGLvbo : public IHgVbo {
public:
	OGLvbo();
	virtual ~OGLvbo();

	inline uint32_t add_data(void* data, uint16_t vertex_count) {
		uint32_t r = m_mem.add_data((T*)data, vertex_count);
		needsUpdate = true;
		return r;
	}

	virtual void use() { use_common(); }

	virtual void draw(uint32_t count, uint32_t vertex_offset, uint32_t idx_offset) { ::draw_vbo<T>(&m_mem, count, vertex_offset, idx_offset); }
	virtual void* getBuffer() { return m_mem.getBuffer(); }
	virtual void clear() { return m_mem.clear(); }
	virtual void setNeedsUpdate(bool x) { needsUpdate = x; }

	inline uint32_t getCount() const { return m_mem.getCount(); }

	static constexpr inline uint8_t Stride() { return sizeof(T); }

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


private:
	void sendToGPU();
	void bind();
	void destroy();

	HgVboMemory<T> m_mem;

	inline void use_common() {
		if (needsUpdate) {
			sendToGPU();
			needsUpdate = false;
		}

		bind();
	}

	struct {
		GLuint vbo_id;
		GLuint vao_id;
	} handle;

	bool needsUpdate;
};

template<typename T>
OGLvbo<T>::OGLvbo()
	:needsUpdate(false)
{
	constexpr VBO_TYPE type = Type();
	m_type = type;
	memset(&handle, 0, sizeof(handle));
	static_assert(Type() != VBO_TYPE_INVALID, "Invalid VBO Type");
}

template<typename T>
OGLvbo<T>::~OGLvbo() {
	destroy();
}

template<typename T>
void OGLvbo<T>::sendToGPU() {
	if (handle.vbo_id == 0) glGenBuffers(1, &handle.vbo_id);
	if (handle.vao_id == 0) glGenVertexArrays(1, &handle.vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, handle.vbo_id);
	glBufferData(GL_ARRAY_BUFFER, m_mem.getCount() * m_mem.Stride(), m_mem.getBuffer(), GL_STATIC_DRAW);

	glBindVertexArray(handle.vao_id);

	glBindBuffer(GL_ARRAY_BUFFER, handle.vbo_id);
	//minimize calls to glVertexAttribPointer, use same format for all meshes in a VBO

	glVertexAttribPointer(L_VERTEX, 3, GL_FLOAT, GL_FALSE, m_mem.Stride(), NULL);
	glEnableVertexAttribArray(L_VERTEX); //enable access to attribute

	constexpr VBO_TYPE vbo_type = Type();

	//Compiler is not optimizing out the switch even though it could
	//maybe write template functions for each part
	switch (vbo_type) {
	case VBO_VC:
	case VBO_COLOR8:
		glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, m_mem.Stride(), (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_COLOR);
		break;
	case VBO_VN:
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, m_mem.Stride(), (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_NORMAL);
		break;
	case VBO_VNU:
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, m_mem.Stride(), (void*)sizeof(vertex));
		glEnableVertexAttribArray(L_NORMAL);
		glVertexAttribPointer(L_UV, 2, GL_UNSIGNED_SHORT, GL_TRUE, m_mem.Stride(), (void*)(sizeof(vertex) + sizeof(normal)));
		glEnableVertexAttribArray(L_UV);
		break;
	case VBO_VNUT:
	{
		size_t offset = sizeof(vertex);
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, m_mem.Stride(), (void*)offset);
		glEnableVertexAttribArray(L_NORMAL);
		offset += sizeof(normal);
		glVertexAttribPointer(L_TANGENT, 4, GL_FLOAT, GL_FALSE, m_mem.Stride(), (void*)offset);
		glEnableVertexAttribArray(L_TANGENT);
		offset += sizeof(tangent); //tangent normals
		glVertexAttribPointer(L_UV, 2, GL_UNSIGNED_SHORT, GL_TRUE, m_mem.Stride(), (void*)offset);
		glEnableVertexAttribArray(L_UV);
		break;
	}
	default:
		fprintf(stderr, "Unknown vbo type:%d\n", vbo_type);
		assert(!"Unknown vbo type");
	}
}

//template<typename T>
//void OGLvbo<T>::use() {
//	if ((_currentVbo == this) && (needsUpdate == false)) return;
//	_currentVbo = this;
//	use_common();
//}

template<typename T>
inline void OGLvbo<T>::bind() {
	glBindBuffer(GL_ARRAY_BUFFER, handle.vbo_id); //is this needed or does the vao_id do this for us?
	glBindVertexArray(handle.vao_id);
}

template<typename T>
void OGLvbo<T>::destroy() {
	if (handle.vbo_id>0) glDeleteBuffers(1, &handle.vbo_id);
	if (handle.vao_id>0) glDeleteBuffers(1, &handle.vao_id);
	m_mem.clear();
}


template<typename T>
inline void draw_vbo(HgVboMemory<T>* vbo, uint32_t count, uint32_t offset, uint32_t idx_offset) {}

//NOTE: THESE ARE INLINE. THEY NEED TO BE IN THE HEADER, NOT CPP
template<>
inline void draw_vbo(HgVboMemory<uint8_t>* vbo, uint32_t indice_count, uint32_t vertex_offset, uint32_t idx_offset) {
	const size_t offset = sizeof(uint8_t)*idx_offset; //offset into indice buffer
	glDrawElementsBaseVertex(GL_TRIANGLES, indice_count, GL_UNSIGNED_BYTE, (void*)offset, vertex_offset);
}

template<>
inline void draw_vbo(HgVboMemory<uint16_t>* vbo, uint32_t indice_count, uint32_t vertex_offset, uint32_t idx_offset) {
	const size_t offset = sizeof(uint16_t)*idx_offset; //offset into indice buffer
	glDrawElementsBaseVertex(GL_TRIANGLES, indice_count, GL_UNSIGNED_SHORT, (void*)offset, vertex_offset);
}


//8 bit index
template<>
inline void OGLvbo<uint8_t>::sendToGPU() {
	if (handle.vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		handle.vbo_id = buf_id;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.vbo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_mem.getCount() * m_mem.Stride(), m_mem.getBuffer(), GL_STATIC_DRAW);
}

//16 bit index
template<>
inline void OGLvbo<uint16_t>::sendToGPU() {
	if (handle.vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		handle.vbo_id = buf_id;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.vbo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_mem.getCount() * m_mem.Stride(), m_mem.getBuffer(), GL_STATIC_DRAW);
}

template<>
inline void OGLvbo<color>::sendToGPU() {
	if (handle.vbo_id == 0) {
		GLuint buf_id;
		glGenBuffers(1, &buf_id);
		handle.vbo_id = buf_id;
		/*
		glGenVertexArrays(1, &vbo->vao_id);
		glBindVertexArray(vbo->vao_id);
		glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(L_COLOR);
		*/
	}

	glBindBuffer(GL_ARRAY_BUFFER, handle.vbo_id);
	glBufferData(GL_ARRAY_BUFFER, m_mem.getCount() * m_mem.Stride(), m_mem.getBuffer(), GL_STATIC_DRAW);
}


//8 bit index
template<>
inline void OGLvbo<uint8_t>::bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.vbo_id);
}

//16 bit index
template<>
inline void OGLvbo<uint16_t>::bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.vbo_id);
}

template<>
inline void OGLvbo<color>::bind() {
	glBindBuffer(GL_ARRAY_BUFFER, handle.vbo_id);
	glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
	glEnableVertexAttribArray(L_COLOR);
}
