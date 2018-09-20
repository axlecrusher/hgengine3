#pragma once

#include <memory>
#include <stdint.h>
#include <HgTypes.h>

#include <vector>

//#include <HgTypes.h>
//#include <oglDisplay.h>
//#include <assert.h>
//#include <RenderBackend.h>


/*	Interleaved vertex layout because it is faster to resize when adding
	more mesh data. New mesh data can just be appened to the end. If it
	were stored VVVNNNCCC, it would be much more difficult to add new
	mesh data as all other mesh data would need to be shifted around. I'm
	unsure of the impact that interleaved data has on rendering performance.
	It doesn't seem any slower than separate VBO buffers for each type.

	We do gain performance from calling glVertexAttribPointer, glBindBuffer
	and glBindVertexArray less.
*/

/*	NOTE: We should try to keep 4 byte alignment.
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
	VBO_INDEX32,
	VBO_COLOR8
};

enum BUFFER_USE_TYPE : uint8_t {
	BUFFER_DRAW_STATIC = 0,
	BUFFER_DRAW_DYNAMIC,
	BUFFER_DRAW_STREAM
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

inline void convert(const vbo_layout_vnut* in, vbo_layout_vn* out) {
	out->v = in->v;
	out->n = in->n;
}

class RenderData;

class IHgVbo {
public:
	IHgVbo() : m_useType(BUFFER_DRAW_STATIC), m_type(VBO_TYPE_INVALID) {}
	virtual ~IHgVbo() {}

	//	Copy data straight into VBO. Ensure that data, is using the correct vbo layout for your data
	virtual uint32_t add_data(void* data, uint32_t count) = 0;

	//	Clear data from RAM. Will not upload new data to GPU until new data is added.
	virtual void clear() = 0;

	virtual void destroy() = 0;

	virtual void use() = 0;

//	virtual void draw(uint32_t count, uint32_t vertex_offset, uint32_t idx_offset) = 0;
	virtual void draw(const RenderData* rd) = 0;

	inline VBO_TYPE VboType() const { return m_type;  }

	inline void UseType(BUFFER_USE_TYPE t) { m_useType = t; }
	inline BUFFER_USE_TYPE UseType() const { return m_useType; }

	virtual void setNeedsUpdate(bool t) = 0;

	virtual void* getBuffer() = 0;
protected:
	VBO_TYPE m_type;
	BUFFER_USE_TYPE m_useType;
};

class HgVboRecord {
public:
	HgVboRecord()
		: m_offset(0), m_count(0)
	{}

	HgVboRecord(std::shared_ptr<IHgVbo>& v, uint32_t o, uint32_t c)
	: m_vbo(v), m_offset(o), m_count(c)
	{}

	std::shared_ptr<IHgVbo>& Vbo() { return m_vbo; }
	uint32_t Offset() const { return m_offset; }
	uint32_t Count() const { return m_count; }

private:
	std::shared_ptr<IHgVbo> m_vbo;
	uint32_t m_offset;
	uint32_t m_count;
};

class VboIndex {
public:
	typedef uint32_t VboIndexType;

	VboIndex() : m_idx(0)
	{}

	~VboIndex();
	VboIndex(const VboIndex& o);
	VboIndex& operator=(VboIndex other) noexcept;
	VboIndex(VboIndex&& other) noexcept;

	VboIndexType Index() const { return m_idx-1; }

	HgVboRecord& VboRec() const;
private:
	void Decrement();
	void Increment();

	explicit VboIndex(VboIndexType idx) : m_idx(idx+1)
	{}

	//void Index(VboIndexType idx) { m_idx = idx+1; }

	VboIndexType m_idx;

	friend class VboManager;
};

class VboManager {
public:
	typedef VboIndex::VboIndexType VboIndexType;
	VboIndex InsertVboRecord(HgVboRecord& vboRec);

	const HgVboRecord& GetVboRecord(const VboIndex& x) const { return m_vboRecords[x.Index()]; }
	HgVboRecord& GetVboRecord(const VboIndex& x) { return m_vboRecords[x.Index()]; }

	static VboManager& Singleton() { return singleton; }

private:
	void IncrementRecordCount(const VboIndex& x);
	void DecrementRecordCount(const VboIndex& x);

	std::vector< HgVboRecord > m_vboRecords;
	std::vector< uint32_t > m_useCount;
	std::vector< uint32_t > m_unusedVboRecords;

	static VboManager singleton;
	friend class VboIndex;
};

namespace HgVbo {
	//Factory Function
	template<typename T>
	std::unique_ptr<IHgVbo> Create();

	template<typename T>
	VboIndex GenerateUniqueFrom(T* data, uint32_t count) {
		//std::shared_ptr<IHgVbo> vbo = Create<T>();
		//auto offset = vbo->add_data(data, count);
		//return HgVboRecord(vbo, offset, count);

		std::shared_ptr<IHgVbo> vbo = Create<T>();
		auto offset = vbo->add_data(data, count);
		return VboManager::Singleton().InsertVboRecord(HgVboRecord(vbo, offset, count));
	}

	template<typename T>
	VboIndex GenerateFrom(T* data, uint32_t count) {
		//static std::shared_ptr<IHgVbo> vbo = Create<T>();
		//auto offset = vbo->add_data(data, count);
		//return HgVboRecord(vbo, offset, count);

		static std::shared_ptr<IHgVbo> vbo = Create<T>();
		auto offset = vbo->add_data(data, count);
		return VboManager::Singleton().InsertVboRecord(HgVboRecord(vbo, offset, count));
	}


}