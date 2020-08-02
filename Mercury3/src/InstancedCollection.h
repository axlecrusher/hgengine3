#pragma once

#include <IUpdatable.h>
#include <UpdatableCollection.h>
#include <algorithm>
#include <HgGPUBuffer.h>
#include <memory>
#include <core/Instancing.h>

#include <OGL/VertexAttributeBuffer.h>

//Stride is how many gpuStructs are used per instance of T
template<typename T, typename gpuStruct, int stride>
class InstancedCollection : public IUpdatableCollection {
public:
	typedef typename SwissArray<T>::iterator iterator;

	struct IdxSort
	{
		size_t hashId;
		uint32_t idx;

		inline bool operator<(const IdxSort& r) const
		{
			return (hashId < r.hashId) && (idx < r.idx);
		}
	};

	InstancedCollection() {
		auto vBuffer = std::make_shared<HgVectorBuffer<gpuStruct>>();

		std::unique_ptr<IGLBufferUse> action = std::make_unique<MatrixVertexAttribute>("ModelMatrix");
		vBuffer->setUseClass(action);

		m_vBuffer = vBuffer;
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

			auto& entity = itr->T::getEntity();
			if (entity.getFlags().destroy)
			{
				remove(*itr);
				continue;
			}

			//m_items can change and b expanded. only store the index to the item.
			//const IdxSort t = { (size_t)entity.getRenderDataPtr().get(),itr.index()};
			//m_updatedItemIdx.push_back(t);
			//m_updatedItemIdx.push_back(itr.index());
			m_updatedItemIdx.push_back(entity.getEntityId());
		}

		//if a thing is enqueued for rendering here, a different part
		//of the engine could come along a delete it before the render
		//happens, causing a crash.
		//Enqueue in second pass after updates.
	}

	struct {
		bool operator()(const EntityRDPair& a, const EntityRDPair& b) const
		{
			if (a.ptr == b.ptr)
			{
				return a.entity < b.entity;
			}
			return a.ptr < b.ptr;
		}
	} orderByRenderData;

	virtual void EnqueueForRender(RenderQueue* queue, HgTime dt) final {
		if (m_updatedItemIdx.empty()) return;

		auto renderDatas = RenderDataTable::Manager().getRenderDataForEntities(m_updatedItemIdx.data(), m_updatedItemIdx.size());

		std::sort(renderDatas.begin(), renderDatas.end(), orderByRenderData);

		////Pointer address is used as the hash id
		////std::unordered_map<size_t, GPUInstanceMetaData<gpuStruct> > instances;

		////Group item instances into groups that can be instance rendered
		////gpuStruct* instanceDataPtr = m_instanceData->getBuffer();

		//auto instanceDataPtr = m_modelMatrices.data();
		//const auto bufferBeginPtr = instanceDataPtr;

		//std::sort(m_updatedItemIdx.begin(), m_updatedItemIdx.end());

		//m_vBuffer->setDataSource(m_modelMatrices);
		//m_vBuffer->setNeedsLoadToGPU(true); //entire vector contents needs to be sent to the GPU
		////vBuffer->setType(BUFFER_TYPE::VERTEX_ATTRIBUTES);

		////Need a way to indicate how the data in vBuffer is supposed to be used.
		////Is it vertex attribute. Is it a sample buffer? How do you use it? How is it tied to a specific attribute?

		//std::vector<EntityIdType> updatedEntities(m_updatedItemIdx.size(), 0);
		//for (int i = 0; i < 0; i++)
		//{
		//	const auto id = getItem(m_updatedItemIdx[i])->getEntity().getEntityId();
		//	updatedEntities[i] = id;
		//}

		//auto rd = RenderDataTable::Manager().getRenderDataForEntities(updatedEntities.data(), updatedEntities.size());

		//std::vector< Instancing::InstancingMetaData > instances;

		//size_t lastHashId = 0;
		//Instancing::InstancingMetaData* ptr = nullptr;

		//for (auto item : m_updatedItemIdx)
		//{
		//	if (item.hashId != lastHashId)
		//	{
		//		lastHashId = item.hashId;
		//		const auto idx = instances.size();
		//		instances.emplace_back( m_vBuffer );
		//		ptr = &instances[idx];
		//		ptr->byteOffset = (instanceDataPtr - bufferBeginPtr) * sizeof(gpuStruct); //compute offset into buffer
		//	}

		//	auto itr = getItem(item.idx);
		//	const HgEntity& entity = itr->getEntity();

		//	auto flags = entity.getFlags();
		//	if (!flags.destroy && !flags.hidden)
		//	{

		//		ptr->instanceCount++;
		//		ptr->renderData = entity.getRenderDataPtr();

		//		itr->T::getInstanceData(instanceDataPtr);
		//		instanceDataPtr += stride;
		//	}
		//}

		//for (auto& itr : instances)
		//{
		//	if (itr.instanceCount > 0)
		//	{
		//		queue->Enqueue(itr);
		//	}
		//}
	}

	inline T* newItem() {
		size_t aSize = m_items.size();
		//T* tmp = &m_items.newItem();
		auto idx = m_items.newItem();
		T* tmp = m_items.at(idx).ptr();
		if (m_items.size() > aSize) {
			//new item created, add more instance data
			//some instance objects can require more than 1 gpuStruct per instance (voxel rain), so use stride as count
			//auto data = std::make_unique<gpuStruct[]>(stride);
			//m_instanceData->AddData(data.get(), stride);
			m_modelMatrices.emplace_back();
		}
		tmp->init();

		////This is an instancing data structure.
		////All instances need to share the same render data.
		//if (renderData == nullptr)
		//	renderData = tmp.getEntity().getRenderDataPtr();

		//tmp.getEntity().setRenderData(renderData);

		return tmp;
	}

	inline std::vector<T*> newItems(int count) {
		std::vector<int> indices(count, 0);
		for (int i = 0; i < count; ++i)
		{
			auto idx = m_items.newItem();
			indices[i] = idx;
			m_items.at(idx)->init();
		}

		m_modelMatrices.resize(m_items.size());

		std::vector<T*> itemPtrs(count, nullptr);
		for (int i = 0; i < count; ++i)
		{
			itemPtrs[i] = m_items.at(indices[i]).ptr();
		}

		return itemPtrs;
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
	//std::vector<uint32_t> m_updatedItemIdx; //has been updated and ready for render
	//std::vector<IdxSort> m_updatedItemIdx; //has been updated and ready for render
	//std::vector<int32_t> m_updatedItemIdx; //has been updated and ready for render
	//std::shared_ptr<HgGPUBuffer<gpuStruct>> m_instanceData;

	std::vector<EntityIdType> m_updatedItemIdx; //has been updated and ready for render


	std::vector<gpuStruct> m_modelMatrices;
	std::shared_ptr<IHgGPUBuffer> m_vBuffer;
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