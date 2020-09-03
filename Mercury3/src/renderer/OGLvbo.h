#pragma once

#include <HgVbo.h>
#include <HgVboMemory.h>

#include <glew.h>
#include <RenderData.h>
#include <core/Instancing.h>
#include <Logging.h>

#include <GpuBufferId.h>
#include <GLBuffer.h>

GLenum hgPrimitiveTypeToGLType(HgEngine::PrimitiveType t);

namespace StackHelpers
{
	//Provides stack calling of glBindBuffer
	struct GLBindBuffer
	{
		GLBindBuffer(GLenum target, GLuint buffer)
			:m_target(target)
		{
			glBindBuffer(m_target, buffer);
		};

		~GLBindBuffer()
		{
			glBindBuffer(m_target, 0);
		}

		GLBindBuffer(const GLBindBuffer&) = delete;
		GLBindBuffer(GLBindBuffer&&) = delete;
		GLBindBuffer& operator=(GLBindBuffer&&) = delete;

		GLenum m_target;
	};
}

template<typename T>
class OGLvbo : public IHgVbo {
public:
	OGLvbo();
	virtual ~OGLvbo();

	inline uint32_t add_data(void* data, uint32_t count) {
		uint32_t r = m_mem.add_data((T*)data, count);
		needsUpdate = true;
		return r;
	}

	virtual void use() { use_common(); }

	virtual void draw(const RenderData* rd)
	{ 
		//if (rd->instanceCount > 0) {
		//	draw_instanced(rd);
		//}
		//else {
			draw_single(rd);
		//}
	}

	virtual void draw(const Instancing::InstancingMetaData* imd)
	{
		if (imd->instanceCount > 0) {
			draw_instanced(imd);
		}
		else {
			assert("This should never happen");
			assert(!"This should never happen");
//			draw_single(rd);
		}
	}

	virtual void* getBuffer() { return m_mem.getBuffer(); }
	virtual void clear() { return m_mem.clear(); }
	virtual void setNeedsUpdate(bool x) { needsUpdate = x; }

	inline uint32_t getCount() const { return m_mem.getCount(); }

	//static constexpr inline uint8_t Stride() { return sizeof(T); }

	static constexpr VBO_TYPE Type() {
		//looks stupid but is compile time evaluated
		if (std::is_same<T, vbo_layout_v>::value) { return VBO_V; }
		if (std::is_same<T, vbo_layout_vc>::value) { return VBO_VC; }
		if (std::is_same<T, vbo_layout_vn>::value) { return VBO_VN; }
		if (std::is_same<T, vbo_layout_vnu>::value) { return VBO_VNU; }
		if (std::is_same<T, vbo_layout_vnut>::value) { return VBO_VNUT; }
		if (std::is_same<T, uint8_t>::value) { return VBO_INDEX8; }
		if (std::is_same<T, uint16_t>::value) { return VBO_INDEX16; }
		if (std::is_same<T, uint32_t>::value) { return VBO_INDEX32; }
		if (std::is_same<T, color8>::value) { return VBO_COLOR8; }
		if (std::is_same<T, color16>::value) { return VBO_COLOR16; }
		return VBO_TYPE_INVALID;
	}


private:
	void sendToGPU();
	void Setup();

	void bind();
	void destroy();

	void draw_single(const RenderData* rd);
	void draw_instanced(const Instancing::InstancingMetaData* imd);

	//returns the GLenum for the type of indices in the vbo
	GLenum indiceType()	{ static_assert(false, "VBO is not an indice vbo");	}

	//instanced vbo rendering using array, not indice
	inline void draw_arrays_instanced(const Instancing::InstancingMetaData* imd)
	{
		const RenderData* rd = imd->renderData;

		const auto& vbo_rec = rd->VertexVboRecord();
		const auto offset = vbo_rec.Offset();
		const auto count = vbo_rec.Count();
		const auto mode = hgPrimitiveTypeToGLType(rd->getPrimitiveType());
		glDrawArraysInstanced(mode, offset, count, imd->instanceCount);
	}

	//instanced vbo rendering using indices
	inline void draw_elements_instanced(const Instancing::InstancingMetaData* imd) {
		const RenderData* rd = imd->renderData;

		const auto& vbo_rec = rd->indexVboRecord();
		const auto idx_offset = vbo_rec.Offset();
		const auto idx_count = vbo_rec.Count();
		const size_t offset = idx_offset * sizeof(T); //offset into indice buffer
		const auto vetexOffest = rd->VertexVboRecord().Offset();
		const auto mode = hgPrimitiveTypeToGLType(rd->getPrimitiveType());
		const auto iType = indiceType();

		StackHelpers::GLBindBuffer bbuffer(GL_ELEMENT_ARRAY_BUFFER, handle.vbo_id.value);
		glDrawElementsInstancedBaseVertex(mode, idx_count, iType, (void*)offset, imd->instanceCount, vetexOffest);
		//glDrawElementsInstancedBaseVertexBaseInstance(mode, idx_count, iType, (void*)offset, imd->instanceCount, vetexOffest, 0);
	}

	//vbo rendering using indices
	inline void draw_elements(const RenderData* rd) {
		const auto indice_count = rd->indexVboRecord().Count();
		const auto vertex_offset = rd->VertexVboRecord().Offset();
		const auto idxOffset = rd->indexVboRecord().Offset();
		const size_t offset = sizeof(uint8_t)*idxOffset; //offset into indice buffer
		const auto iType = indiceType();
		const auto mode = hgPrimitiveTypeToGLType(rd->getPrimitiveType());

		StackHelpers::GLBindBuffer bbuffer(GL_ELEMENT_ARRAY_BUFFER, handle.vbo_id.value);
		glDrawElementsBaseVertex(mode, indice_count, iType, (void*)offset, vertex_offset);
	}

	void sendIndicesToGPU() {
		const auto count = m_mem.getCount();
		const auto stride = m_mem.Stride();
		const auto useType = VboUseage(m_useType);

		StackHelpers::GLBindBuffer bbuffer(GL_ELEMENT_ARRAY_BUFFER, handle.vbo_id.value);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count*stride, m_mem.getBuffer(), useType);
	}

	HgVboMemory<T> m_mem;
	GLuint m_vboType;

	inline void use_common() {
		if (handle.vbo_id == 0) {
			handle.vbo_id.Init();
		}

		//Setup();

		if (needsUpdate) {
			sendToGPU();
			needsUpdate = false;
		}

		Setup();
		bind();
	}


	struct {
		GpuBufferId vbo_id;
		//GLVaoId vao_id;
	} handle;

	bool needsUpdate;


};

template<typename T>
OGLvbo<T>::OGLvbo()
	:needsUpdate(false), IHgVbo()
{
	constexpr VBO_TYPE type = Type();
	m_type = type;
	m_stride = sizeof(T);
	memset(&handle, 0, sizeof(handle));
	static_assert(Type() != VBO_TYPE_INVALID, "Invalid VBO Type");
}

template<typename T>
OGLvbo<T>::~OGLvbo() {
	destroy();
}

inline GLenum VboUseage(BUFFER_USE_TYPE t) {
	switch (t) {
	case BUFFER_DRAW_STATIC:
		return GL_STATIC_DRAW;
	case BUFFER_DRAW_DYNAMIC:
		return GL_DYNAMIC_DRAW;
	case BUFFER_DRAW_STREAM:
		return GL_STREAM_DRAW;
	}
	return 0;
}

template<typename T>
void OGLvbo<T>::sendToGPU()
{
	//bool updateOnly = true;
	//if (handle.vbo_id == 0) {
	//	handle.vbo_id.Init();
	//	updateOnly = false;
	//}

	const auto useType = VboUseage(m_useType);

	StackHelpers::GLBindBuffer bbuffer(GL_ARRAY_BUFFER, handle.vbo_id.value);

	const GLsizei size = (GLsizei)m_mem.getSizeBytes();

	//if (updateOnly) {
	//	//try orphaning buffer
	//	glBufferData(GL_ARRAY_BUFFER, size, NULL, useType);
	//	glBufferSubData(GL_ARRAY_BUFFER, 0, size, m_mem.getBuffer());
	//	//glBufferData(GL_ARRAY_BUFFER, m_mem.getCount() * m_mem.Stride(), m_mem.getBuffer(), useType);
	//}
	//else {
	//	glBufferData(GL_ARRAY_BUFFER, size, m_mem.getBuffer(), useType);
	//}

	glBufferData(GL_ARRAY_BUFFER, size, m_mem.getBuffer(), useType);
	//Setup();
}

template<typename T>
void OGLvbo<T>::Setup()
{
	const GLsizei stride = (GLsizei)m_mem.Stride();

	//glBindVertexArray(handle.vao_id);

	StackHelpers::GLBindBuffer bbuffer(GL_ARRAY_BUFFER, handle.vbo_id.value);
	//minimize calls to glVertexAttribPointer, use same format for all meshes in a VBO

	glVertexAttribPointer(L_VERTEX, 3, GL_FLOAT, GL_FALSE, stride, NULL);
	glEnableVertexAttribArray(L_VERTEX); //enable access to attribute
	glVertexAttribDivisor(L_VERTEX, 0);

	constexpr VBO_TYPE vbo_type = Type();

	//Compiler is not optimizing out the switch even though it could
	//maybe write template functions for each part
	switch (vbo_type) {
	case VBO_V:
	{
		break;
	}
	case VBO_VC:
	case VBO_COLOR8:
	{
		size_t offset = sizeof(vertex);
		glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)offset);
		glEnableVertexAttribArray(L_COLOR);
		glVertexAttribDivisor(L_COLOR, 0);
		break;
	}
	case VBO_VN:
	{
		size_t offset = sizeof(vertex);
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
		glEnableVertexAttribArray(L_NORMAL);
		glVertexAttribDivisor(L_NORMAL, 0);
		break;
	}
	case VBO_VNU:
	{
		size_t offset = sizeof(vertex);
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
		glEnableVertexAttribArray(L_NORMAL);
		glVertexAttribDivisor(L_NORMAL, 0);
		offset += sizeof(normal);
		glVertexAttribPointer(L_UV, 2, GL_UNSIGNED_SHORT, GL_TRUE, stride, (void*)offset);
		glEnableVertexAttribArray(L_UV);
		glVertexAttribDivisor(L_UV, 0);
		break;
	}
	case VBO_VNUT:
	{
		size_t offset = sizeof(vertex);
		glVertexAttribPointer(L_NORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)offset);
		glEnableVertexAttribArray(L_NORMAL);
		glVertexAttribDivisor(L_NORMAL, 0);
		offset += sizeof(normal);
		glVertexAttribPointer(L_TANGENT, 4, GL_FLOAT, GL_FALSE, stride, (void*)offset);
		glEnableVertexAttribArray(L_TANGENT);
		glVertexAttribDivisor(L_TANGENT, 0);
		offset += sizeof(tangent); //tangent normals
		glVertexAttribPointer(L_UV, 2, GL_UNSIGNED_SHORT, GL_TRUE, stride, (void*)offset);
		glEnableVertexAttribArray(L_UV);
		glVertexAttribDivisor(L_UV, 0);
		break;
	}
	default:
		LOG_ERROR("Unknown vbo type:%d", vbo_type);
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
	//glBindBuffer(GL_ARRAY_BUFFER, handle.vbo_id.value); //VAO does not bind GL_ARRAY_BUFFER.
	//glBindVertexArray(handle.vao_id);
}

template<typename T>
void OGLvbo<T>::destroy() {
	handle.vbo_id.Destroy();
	//handle.vao_id.Destroy();
	m_mem.clear();
}

//default 
template<typename T>
inline void OGLvbo<T>::draw_single(const RenderData* rd) { }

//default to drawing using arrays. Indice drawing needs its own templates
template<typename T>
inline void OGLvbo<T>::draw_instanced(const Instancing::InstancingMetaData* imd) { draw_arrays_instanced(imd); }

//NOTE: THESE ARE INLINE. THEY NEED TO BE IN THE HEADER, NOT CPP
template<>
inline void OGLvbo<uint8_t>::draw_single(const RenderData* rd) { draw_elements(rd); }

template<>
inline void OGLvbo<uint16_t>::draw_single(const RenderData* rd) { draw_elements(rd); }

template<>
inline void OGLvbo<uint32_t>::draw_single(const RenderData* rd) { draw_elements(rd); }

template<>
inline void OGLvbo<uint8_t>::draw_instanced(const Instancing::InstancingMetaData* imd) { draw_elements_instanced(imd); }

template<>
inline void OGLvbo<uint16_t>::draw_instanced(const Instancing::InstancingMetaData* imd) { draw_elements_instanced(imd); }

template<>
inline void OGLvbo<uint32_t>::draw_instanced(const Instancing::InstancingMetaData* imd) { draw_elements_instanced(imd); }

template<>
inline GLenum OGLvbo<uint8_t>::indiceType() { return GL_UNSIGNED_BYTE; }

template<>
inline GLenum OGLvbo<uint16_t>::indiceType() { return GL_UNSIGNED_SHORT; }

template<>
inline GLenum OGLvbo<uint32_t>::indiceType() { return GL_UNSIGNED_INT; }

//8 bit index
template<>
inline void OGLvbo<uint8_t>::sendToGPU() { sendIndicesToGPU(); }

//16 bit index
template<>
inline void OGLvbo<uint16_t>::sendToGPU() { sendIndicesToGPU(); }

//32 bit index
template<>
inline void OGLvbo<uint32_t>::sendToGPU() { sendIndicesToGPU(); }

template<>
inline void OGLvbo<color8>::sendToGPU() { sendIndicesToGPU(); }

//8 bit index
template<>
inline void OGLvbo<uint8_t>::Setup() { }

//16 bit index
template<>
inline void OGLvbo<uint16_t>::Setup() { }

//32 bit index
template<>
inline void OGLvbo<uint32_t>::Setup() { }

template<>
inline void OGLvbo<color8>::Setup() { }


//8 bit index
template<>
inline void OGLvbo<uint8_t>::bind() { }

//16 bit index
template<>
inline void OGLvbo<uint16_t>::bind() { }

//16 bit index
template<>
inline void OGLvbo<uint32_t>::bind() { }

template<>
inline void OGLvbo<color8>::bind() {
	StackHelpers::GLBindBuffer bbuffer(GL_ARRAY_BUFFER, (GLuint)handle.vbo_id.value);
	glVertexAttribPointer(L_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, NULL);
	glEnableVertexAttribArray(L_COLOR);
}
