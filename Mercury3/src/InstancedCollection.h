#pragma once

#include <IUpdatable.h>
#include <UpdatableCollection.h>

template<typename T, typename gpuStruct, int stride>
class InstancedCollection : public IUpdatableCollection {
public:
	typedef typename SwissArray<T>::iterator iterator;

	InstancedCollection() {
	}

	//InstancedCollection(uint32_t reserve) : m_items(reserve) {
	//}

	virtual void update(HgTime dtime) final {
		if (empty()) return;
		if (dtime.msec() == 0) return;

		//new instances could be added after this update, so keep track of how many
		//instances were updated so that we don't render newly added but not updated
		//instances that have been created.
		m_instanceCount = 0; 

		gpuStruct* instanceDataPtr = m_instanceData.getBuffer();
		for (auto itr = begin(); itr != end(); itr++) {
			itr->T::update(dtime, instanceDataPtr); //avoid vtable lookup
			if (!itr->getElement().flags.destroy) {
				instanceDataPtr += stride;
				m_instanceCount++;
			}
			//if a thing is enqueued for rendering here, a different part
			//of the engine could come along a delete it before the render
			//happens, causing a crash.
			//Enqueue in second pass after updates.
		}
	}

	virtual void EnqueueForRender() final {
		if (empty()) return;

//		size_t instanceCount = m_items.count();
		//renderData->instanceCount = instanceCount;
		renderData->instanceCount = (uint32_t)m_instanceCount;
		renderData->gpuBuffer = &m_instanceData;

		//enqueue first one found. hope it shares render data
		for (auto itr = begin(); itr != end(); itr++) {
			auto& e = itr->getElement();
			if (!e.flags.hidden) {
				Renderer::Enqueue(e);
				return;
			}
		}
	}

	inline T& newItem() {
		size_t aSize = m_items.size();
		T& tmp = m_items.newItem();
		if (m_items.size() > aSize) {
			//new item created, add more instance data
			auto data = std::make_unique<gpuStruct[]>(stride);
			m_instanceData.AddData(data.get(), stride);
		}
		return tmp;
	}

	inline void remove(const T& x) {
		return m_items.remove(x);
	}

	inline iterator erase(const iterator& itr) {
		return m_items.erase(itr);
	}

	inline iterator begin()
	{
		return m_items.begin();
	}

	inline iterator end()
	{
		return m_items.end();
	}

	//const_iterator begin() const
	//{
	//	size_t i = 0;
	//	for (; i < m_used.size(); ++i) {
	//		if (m_used[i]) break;
	//	}
	//	return iterator(i, this);
	//}

	//const_iterator end() const
	//{
	//	return iterator(m_entities.size(), this);
	//}

	inline bool empty() const { return m_items.empty(); }

	static InstancedCollection<T, gpuStruct, stride>& Collection() {
		static InstancedCollection<T, gpuStruct, stride> collection;
		return collection;
	}

	std::shared_ptr<RenderData> renderData;

private:
	size_t m_instanceCount;
	SwissArray<T> m_items;
	HgGPUBuffer<gpuStruct> m_instanceData;
};

template<typename gpu_structure>
class IUpdatableInstance : public IUpdatable {
public:
	virtual void update(HgTime tdelta, gpu_structure* instanceData) = 0;
};