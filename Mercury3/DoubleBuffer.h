#pragma once

#include <atomic>

template <typename T>
class DoubleBuffer {
public:
	DoubleBuffer() : m_frontBuffer(nullptr), m_backBuffer(nullptr)
	{}

	~DoubleBuffer() {
		cleanup();
	}

	T* frontBuffer() { return m_frontBuffer; }
	T* backBuffer() { return m_backBuffer; }

	void swap() {
		m_backBuffer.exchange(m_frontBuffer.exchange(m_backBuffer));
	}
	
	void allocate(size_t s) { 
		cleanup();
		m_frontBuffer = new T[s]();
		m_backBuffer = new T[s]();
	}
private:
	void cleanup() {
		if (m_frontBuffer) delete[] m_frontBuffer;
		if (m_backBuffer) delete[] m_backBuffer;

		m_frontBuffer = m_backBuffer = nullptr;
	}
	std::atomic<T*> m_frontBuffer;
	std::atomic<T*> m_backBuffer;
};