#pragma once

#include <atomic>

template <typename T>
class DoubleBuffer {
public:
	DoubleBuffer() : m_frontBuffer2(nullptr), m_frontBuffer1(nullptr), m_backBuffer(nullptr), m_size(0)
	{}

	~DoubleBuffer() {
		cleanup();
	}

	//Swap front buffer into isolated pointer
	void swapFrontBuffers() {
		m_frontBuffer1.exchange(m_frontBuffer2.exchange(m_frontBuffer1));
	}
	//Isolated buffer not touched by swap() or frontBuffer
	T* isolatedFrontBuffer() const {
		return m_frontBuffer2;
	}

	T* frontBuffer() const { return m_frontBuffer1; } //readonly
	T* backBuffer() { return m_backBuffer; } //writeable

	//swap front and back buffer
	void swap() {
		m_backBuffer.exchange(m_frontBuffer1.exchange(m_backBuffer));
	}
	
	void allocate(size_t s) { 
		cleanup();
		m_frontBuffer2 = new T[s]();
		m_frontBuffer1 = new T[s]();
		m_backBuffer = new T[s]();
	}

	uint32_t size() const { return m_size; }

private:
	void cleanup() {
		if (m_frontBuffer1) delete[] m_frontBuffer1;
		if (m_frontBuffer2) delete[] m_frontBuffer2;
		if (m_backBuffer) delete[] m_backBuffer;

		m_frontBuffer2 = m_frontBuffer1 = m_backBuffer = nullptr;
	}
	std::atomic<T*> m_frontBuffer1, m_frontBuffer2;
	std::atomic<T*> m_backBuffer;
	uint32_t m_size;
};