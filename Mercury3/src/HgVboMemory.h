#pragma once

#include <stdint.h>
#include <assert.h>
#include <vector>

template<typename T>
class HgVboMemory
{
public:
	HgVboMemory();
	~HgVboMemory();

	//returns the offset in the buffer where the added chunk of data begins
	uint32_t add_data(const T* data, uint32_t count);
	void clear() { m_buffer.clear(); }

	inline T* getBuffer() { return m_buffer.data(); }
	inline uint32_t getCount() const { return (uint32_t)m_buffer.size(); }
	static constexpr inline size_t Stride() { return sizeof(T); }

	inline size_t getSizeBytes() const { return getCount() * sizeof(T); }

private:
	std::vector<T> m_buffer;
	//T* HgVboMemory::resize(uint32_t count);

	//T* buffer;
	//uint32_t count;
};

template<typename T>
HgVboMemory<T>::HgVboMemory()
	//:buffer(nullptr), count(0)
{
}

template<typename T>
HgVboMemory<T>::~HgVboMemory() {
	clear();
}

//template<typename T>
//T* HgVboMemory<T>::resize(uint32_t count) {
//	T* buf = (T*)realloc(buffer, count * sizeof(T));
//	assert(buf != NULL);
//	buffer = buf;
//
//	return buf;
//}

//template<typename T>
//void HgVboMemory<T>::clear() {
//	if (buffer != nullptr) free(buffer);
//	buffer = NULL;
//	count = 0;
//}

template<typename T>
uint32_t HgVboMemory<T>::add_data(const T* data, uint32_t count) {
	const auto offset = getCount();
	m_buffer.resize(offset + count);

	T* buf = getBuffer() + offset;
	memcpy(buf, data, sizeof(T)*count);

	//T* buf = buffer = resize(count + vertex_count);
	//buf = buf + count;

	//memcpy(buf, data, sizeof(T)*vertex_count);

	//uint32_t offset = count;
	//count += vertex_count;

	return offset;
}
