#pragma once

#include <IUpdatable.h>
#include <UpdatableCollection.h>
#include <algorithm>

template<typename T, typename gpuStruct, int stride>
class InstancedCollection : public IUpdatableCollection {
public:
	typedef typename SwissArray<T>::iterator iterator;

	InstancedCollection() {
		m_instanceCount = 0;
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
			if (!itr->getEntity().flags.destroy) {
				instanceDataPtr += stride;
				m_instanceCount++;
			}
			//if a thing is enqueued for rendering here, a different part
			//of the engine could come along a delete it before the render
			//happens, causing a crash.
			//Enqueue in second pass after updates.
		}
	}

	virtual void EnqueueForRender(RenderQueue* queue) final {
		if (empty() || m_instanceCount<1) return;

		auto itr = std::find_if(begin(), end(), [](auto& x) {
			auto& e = x.getEntity();
			return e.flags.hidden == false;
		});

		if (itr != end())
		{
			auto &e = itr->getEntity();
			auto& rd = e.getRenderDataPtr();

			//render data doesn't copy properly right now. just take the pointer
			//*renderData = *rd;
			renderData = rd;

			renderData->instanceCount = (uint32_t)m_instanceCount;
			renderData->gpuBuffer = &m_instanceData;
			renderData->instanceCount = (uint32_t)m_instanceCount;
			renderData->gpuBuffer = &m_instanceData;

			queue->Enqueue(renderData);
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
		tmp.init();

		//This is an instancing data structure.
		//All instances need to share the same render data.
		if (renderData == nullptr)
			renderData = tmp.getEntity().getRenderDataPtr();

		tmp.getEntity().setRenderData(renderData);

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