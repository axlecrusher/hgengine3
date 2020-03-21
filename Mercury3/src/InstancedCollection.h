#pragma once

#include <IUpdatable.h>
#include <UpdatableCollection.h>
#include <algorithm>

namespace Instancing
{
	struct GPUTransformationMatrix
	{
		float matrix[16];
	};
}

//Structure to contain the data required for instance rendering
template<typename gpuStruct>
struct GPUInstanceMetaData
{
	GPUInstanceMetaData()
		:instanceCount(0)//, entityPtr(nullptr)
	{
		instanceData = std::make_shared<HgGPUBufferSegment<gpuStruct>>();
	}
	uint32_t instanceCount;
	RenderDataPtr renderData;
	//HgEntity* entityPtr;

	std::shared_ptr< HgGPUBufferSegment<gpuStruct> > instanceData;
};

//Stride is how many gpuStructs are used per instance of T
template<typename T, typename gpuStruct, int stride>
class InstancedCollection : public IUpdatableCollection {
public:
	typedef typename SwissArray<T>::iterator iterator;

	InstancedCollection() {
		m_instanceData = std::make_shared< HgGPUBuffer<gpuStruct> >();
	}

	virtual void update(HgTime dtime) final {
		if (empty()) return;
		if (dtime.msec() == 0) return;

		//new instances could be added after this update, so keep track of how many
		//instances were updated so that we don't render newly added but not updated
		//instances that have been created.
		m_updatedItemIdx.clear();
		m_updatedItemIdx.reserve(m_items.count());

		for (auto itr = begin(); itr != end(); itr++) {
			//auto& i = *itr;
			itr->T::update(dtime); //avoid vtable lookup

			//m_items can change and b expanded. only store the index to the item.
			m_updatedItemIdx.push_back(itr.index());
		}

		//if a thing is enqueued for rendering here, a different part
		//of the engine could come along a delete it before the render
		//happens, causing a crash.
		//Enqueue in second pass after updates.
	}

	virtual void EnqueueForRender(RenderQueue* queue, HgTime dt) final {
		if (m_updatedItemIdx.empty()) return;

		//Pointer address is used as the hash id
		std::unordered_map<size_t, GPUInstanceMetaData<gpuStruct> > instances;

		//Group item instances into groups that can be instance rendered
		gpuStruct* instanceDataPtr = m_instanceData->getBuffer();
		for (auto idx : m_updatedItemIdx)
		{
			auto itr = getItem(idx);
			const HgEntity& entity = itr->getEntity();

			auto flags = entity.getFlags();
			if (!flags.destroy && !flags.hidden)
			{
				//use the pointer address as the hash id
				const size_t hashId = (size_t)entity.renderData();
				auto& tmp = instances[hashId];

				if (tmp.instanceData->getBufferPtr() == nullptr)
				{
					tmp.instanceData->setBuffer(instanceDataPtr);
				}

				tmp.renderData = entity.getRenderDataPtr();
				tmp.instanceCount++;

				itr->T::getInstanceData(instanceDataPtr);
				instanceDataPtr += stride;
			}
		}

		for (auto& itr : instances)
		{
			RenderDataPtr& rd = itr.second.renderData;
			rd->instanceCount = itr.second.instanceCount;
			itr.second.instanceData->setCount(itr.second.instanceCount*stride);
			itr.second.instanceData->setNeedsLoadToGPU(true);

			rd->gpuBuffer = itr.second.instanceData;
			queue->Enqueue(rd);
		}
	}

	inline T& newItem() {
		size_t aSize = m_items.size();
		T& tmp = m_items.newItem();
		if (m_items.size() > aSize) {
			//new item created, add more instance data
			//some instance objects can require more than 1 gpuStruct per instance (voxel rain), so use stride as count
			auto data = std::make_unique<gpuStruct[]>(stride);
			m_instanceData->AddData(data.get(), stride);
		}
		tmp.init();

		////This is an instancing data structure.
		////All instances need to share the same render data.
		//if (renderData == nullptr)
		//	renderData = tmp.getEntity().getRenderDataPtr();

		//tmp.getEntity().setRenderData(renderData);

		return tmp;
	}

	inline void remove(const T& x) {
		return m_items.remove(x);
	}

	inline auto getItem(uint32_t idx) { return m_items.at(idx); }

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

	//std::shared_ptr<RenderData> renderData;

private:
	//size_t m_instanceCount;
	SwissArray<T> m_items;
	std::vector<uint32_t> m_updatedItemIdx; //has been updated and ready for render
	std::shared_ptr<HgGPUBuffer<gpuStruct>> m_instanceData;
};

template<typename gpu_structure>
class IUpdatableInstance : public IUpdatable {
public:
	virtual void getInstanceData(gpu_structure* instanceData) = 0;
};

template<typename gpu_structure>
class DoDUpdatableInstance : public IUpdatable {
public:
	virtual void getInstanceData(gpu_structure* instanceData) = 0;
};