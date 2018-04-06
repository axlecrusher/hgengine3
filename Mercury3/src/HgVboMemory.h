#pragma once

#include <stdint.h>

template<typename T>
class HgVboMemory
{
public:
	HgVboMemory();
	~HgVboMemory();

	uint32_t add_data(T* data, uint16_t vertex_count);
	void clear();

	inline T* getBuffer() { return buffer; }
	inline uint32_t getCount() const { return count; }
	static constexpr inline uint8_t Stride() { return sizeof(T); }

private:
	T* HgVboMemory::resize(uint32_t count);

	T* buffer;
	uint32_t count;
};

template<typename T>
HgVboMemory<T>::HgVboMemory()
	:buffer(nullptr), count(0)
{
}

template<typename T>
HgVboMemory<T>::~HgVboMemory() {
	clear();
}

template<typename T>
T* HgVboMemory<T>::resize(uint32_t count) {
	T* buf = (T*)realloc(buffer, count * sizeof(T));
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
uint32_t HgVboMemory<T>::add_data(T* data, uint16_t vertex_count) {
	T* buf = buffer = resize(count + vertex_count);
	buf = buf + count;

	memcpy(buf, data, sizeof(T)*vertex_count);

	uint32_t offset = count;
	count += vertex_count;

	return offset;
}
