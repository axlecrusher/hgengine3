#pragma once

#include <memory>
#include <HgVboMemory.h>
#include <HgVbo.h>

class IHgGPUBuffer;

//Graphics API implementation
class IGPUBufferImpl {
public:
	virtual ~IGPUBufferImpl() {}

	virtual void SendToGPU(const IHgGPUBuffer* bufferObject) = 0;
	virtual void Bind(uint16_t textureLocation) = 0;
};

class IHgGPUBuffer {
public:
	IHgGPUBuffer() : m_needsUpdate(false),
		m_useType(BUFFER_DRAW_STATIC), m_buffer(nullptr)
	{
		m_sizeBytes = 0;
		m_gpuImpl = from_api_type();
	}
	virtual ~IHgGPUBuffer() {
	}

	inline void NeedsUpdate(bool needsUpdate) { m_needsUpdate = needsUpdate; }
	inline bool NeedsUpdate() const { return m_needsUpdate; }

	inline void UseType(BUFFER_USE_TYPE t) { m_useType = t; }
	inline BUFFER_USE_TYPE UseType() const { return m_useType; }

	inline uint32_t sizeBytes() const { return m_sizeBytes; }

	//inline void Use() {
	//	if (NeedsUpdate()) {
	//		m_gpuImpl->SendToGPU(this);
	//		NeedsUpdate(false);
	//	}
	//	m_gpuImpl->Bind();
	//}

	inline IGPUBufferImpl* apiImpl() const { return m_gpuImpl.get(); }

	inline const void* getBufferPtr() const { return m_buffer; }

protected:
	static std::unique_ptr<IGPUBufferImpl> from_api_type();
	uint32_t m_sizeBytes;
	void* m_buffer;

private:
	bool m_needsUpdate;
	BUFFER_USE_TYPE m_useType;
	std::unique_ptr<IGPUBufferImpl> m_gpuImpl;
};

template<typename T>
class HgGPUBuffer : public IHgGPUBuffer {

public:

	inline void AddData(const T* data, uint32_t count) {
		m_memory.add_data(data, count);
		m_sizeBytes = m_memory.getCount() * sizeof(T);
		NeedsUpdate(true);
		m_buffer = m_memory.getBuffer();
	}

	inline void Clear() {
		m_memory.clear();
	}

	inline T* getBuffer() { return m_memory.getBuffer(); }

	inline size_t AllocatedSize() const { return m_memory.getCount(); }

private:
	HgVboMemory<T> m_memory;
};


//implementation of IHgGPUBuffer that uses a portion of a HgGPUBuffer
template<typename T>
class HgGPUBufferSegment : public IHgGPUBuffer {

public:
	inline void setBuffer(T* ptr) { m_buffer = ptr; }

	//set the size of the buffer based on the number of items
	inline void setCount(uint32_t count) { m_sizeBytes = count * sizeof(T); }
};