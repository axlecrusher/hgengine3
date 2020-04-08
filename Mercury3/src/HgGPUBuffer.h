#pragma once

#include <memory>
#include <HgVboMemory.h>
//#include <HgVbo.h>
#include <core/Enumerations.h>
#include <HgShader.h>

//forward declirations
class IHgGPUBuffer;
namespace Instancing
{
	struct InstancingMetaData;
}


//Graphics API implementation
class IGPUBufferImpl {
public:
	virtual ~IGPUBufferImpl() {}

	virtual void SendToGPU(const IHgGPUBuffer* bufferObject) = 0;
	virtual void Bind(uint16_t textureLocation) = 0;
};

class IGLBufferUse
{
public:
	virtual ~IGLBufferUse()
	{}

	virtual void SendToGPU(const IHgGPUBuffer* bufferObject) = 0;
	virtual void Setup(const Instancing::InstancingMetaData& imd, const HgShader& shader) = 0;

	inline void setNeedSetup(bool t) { m_needSetup = t; }
	inline bool getNeedsSetup() const { return m_needSetup; }
protected:
	bool m_needSetup;
};

class IHgGPUBuffer {
public:
	IHgGPUBuffer() : m_needsUpdate(false),
		m_useType(BUFFER_DRAW_STATIC), m_buffer(nullptr)
	{
		m_sizeBytes = 0;
		m_type = BUFFER_TYPE::VERTEX_ATTRIBUTES;
		//m_gpuImpl = from_api_type();
	}
	virtual ~IHgGPUBuffer() {
	}

	//Set to true if the data needs to be updated on the GPU
	inline void setNeedsLoadToGPU(bool needsUpdate) { m_needsUpdate = needsUpdate; }

	//returns true if the data needs to be loaded to the GPU
	inline bool NeedsLoadToGPU() const { return m_needsUpdate; }

	void sendToGPU();

	inline void UseType(BUFFER_USE_TYPE t) { m_useType = t; }
	inline BUFFER_USE_TYPE UseType() const { return m_useType; }

	inline uint32_t sizeBytes() const { return m_sizeBytes; }

	//inline IGPUBufferImpl* apiImpl() const { return m_gpuImpl.get(); }

	inline void setUseClass(std::unique_ptr<IGLBufferUse>& useCase) { m_bufferUse = std::move(useCase); }
	inline IGLBufferUse* getUseClass() const { return m_bufferUse.get(); }

	inline const void* getBufferPtr() const { return m_buffer; }

	inline void setType(BUFFER_TYPE type) { m_type = type; }
	inline BUFFER_TYPE getType() const { return m_type; }

	template<typename T>
	inline void setDataSource(const std::vector<T>& v)
	{
		m_buffer = (void*)v.data();
		m_sizeBytes = sizeof(T) * v.size();
		//setCount(v.size());
	}

protected:
	//static std::unique_ptr<IGPUBufferImpl> from_api_type();
	uint32_t m_sizeBytes;
	void* m_buffer; //pointer to data buffer

private:
	bool m_needsUpdate;
	BUFFER_USE_TYPE m_useType;
	BUFFER_TYPE m_type;

	//uint16_t m_glBufferHandle;
	std::unique_ptr<IGLBufferUse> m_bufferUse;

	//std::unique_ptr<IGPUBufferImpl> m_gpuImpl;
};

template<typename T>
class HgGPUBuffer : public IHgGPUBuffer {

public:

	inline void AddData(const T* data, uint32_t count) {
		m_memory.add_data(data, count);
		m_sizeBytes = m_memory.getCount() * sizeof(T);
		setNeedsLoadToGPU(true);
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


//Implementation of IHgGPUBuffer that simply points to the data array of an std::vector
template<typename T>
class HgVectorBuffer : public IHgGPUBuffer {

public:
private:
	//set the size of the buffer based on the number of items
	//inline void setCount(uint32_t count) { m_sizeBytes = count * sizeof(T); }
};