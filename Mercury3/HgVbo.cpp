#include <HgVbo.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <string.h>
#include <RenderBackend.h>

#include <OGLvbo.h>
#include <atomic>

//static void* _currentVbo;

VboManager VboManager::singleton;
//static HgVboRecord emptyRec; // >:(
//
//VboIndex VboManager::InsertVboRecord(HgVboRecord& vboRec) {
//	VboIndexType index;
//	if (m_unusedVboRecords.size() > 0) {
//		index = m_unusedVboRecords.back();
//		m_unusedVboRecords.pop_back();
//		m_vboRecords[index] = vboRec;
//		m_useCount[index] = 1;
//		//fprintf(stderr, "Reusing index %d\n", index);
//	}
//	else {
//		index = (VboIndexType)m_vboRecords.size();
//		m_vboRecords.push_back(vboRec);
//		m_useCount.push_back(1);
//		//fprintf(stderr, "New index %d\n", index);
//	}
//	return VboIndex(index);
//}
//
//void VboManager::IncrementRecordCount(const VboIndex& x) {
//	auto idx = x.Index();
//	if (m_useCount.size() > idx) {
//		m_useCount[idx]++;
//	}
//}
//
//void VboManager::DecrementRecordCount(const VboIndex& x) {
//	auto idx = x.Index();
//	if (m_useCount.size() > idx) {
//		m_useCount[idx]--;
//		if (m_useCount[idx] == 0) {
//			m_unusedVboRecords.push_back(idx);
//			//fprintf(stderr, "reference count reached zero\n");
//			m_vboRecords[idx] = emptyRec;
//			//run cleanup routine here
//		}
//	}
//}

template<typename T>
static std::unique_ptr<IHgVbo> vbo_from_api_type() {
	switch (RENDERER()->Type()) {
	case OPENGL:
		return std::move( std::make_unique< OGLvbo<T> >() );
		break;
	default:
		return nullptr;
	}
}


namespace HgVbo {
	template<>
	std::unique_ptr<IHgVbo> Create<vbo_layout_v>() {
		return std::move(vbo_from_api_type<vbo_layout_v>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<vector3f>() {
		return std::move(vbo_from_api_type<vbo_layout_v>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<vbo_layout_vc>() {
		return std::move(vbo_from_api_type<vbo_layout_vc>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<vbo_layout_vn>() {
		return std::move(vbo_from_api_type<vbo_layout_vn>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<vbo_layout_vnu>() {
		return std::move(vbo_from_api_type<vbo_layout_vnu>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<vbo_layout_vnut>() {
		return std::move(vbo_from_api_type<vbo_layout_vnut>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<uint8_t>() {
		return std::move(vbo_from_api_type<uint8_t>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<uint16_t>() {
		return std::move(vbo_from_api_type<uint16_t>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<uint32_t>() {
		return std::move(vbo_from_api_type<uint32_t>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<color8>() {
		return std::move(vbo_from_api_type<color8>());
	}

	template<>
	std::unique_ptr<IHgVbo> Create<color16>() {
		return std::move(vbo_from_api_type<color16>());
	}
}
//
//VboIndex::~VboIndex() {
//	Decrement();
//}
//
//VboIndex::VboIndex(const VboIndex& o) : m_idx(o.m_idx) {
//	Increment();
//}
//
//VboIndex::VboIndex(VboIndex&& other) noexcept
//	: m_idx(std::exchange(other.m_idx, 0))
//{
//}
//
//VboIndex& VboIndex::operator=(VboIndex other) noexcept
//{
//	//I don't think decrement or increment need to happen here.
//	//Take it from the copied argument
//	std::swap(m_idx, other.m_idx);
//	return *this;
//}
//
//void VboIndex::Decrement() {
//	if (m_idx != 0) {
//		VboManager::Singleton().DecrementRecordCount(*this);
//	}
//}
//
//void VboIndex::Increment() {
//	if (m_idx != 0) {
//		VboManager::Singleton().IncrementRecordCount(*this);
//	}
//}
//
//HgVboRecord& VboIndex::VboRec() const {
//	if (m_idx != 0) {
//		return VboManager::Singleton().GetVboRecord(*this);
//	}
//	return emptyRec; //eewwww
//}

void* HgVboRecord::getBuffer() const {
	uint8_t* data = (uint8_t*)m_vbo->getBuffer();
	data += m_vbo->Stride() * Offset();
	return data;
}