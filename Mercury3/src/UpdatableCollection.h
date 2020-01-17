#pragma once

#include <SwissArray.h>
#include <HgTimer.h>

#include <RenderBackend.h>

class IUpdatableCollection {
public:
	virtual void update(HgTime dtime) = 0;
	virtual void EnqueueForRender(RenderQueue* queue, HgTime dt) = 0;
};

using IUpdatableCollectionPtr = std::shared_ptr<IUpdatableCollection>;

/* This is for data oriented processing. Large collections of objects can
be stored contiguously in memory. Updates benefit from cache hits.*/
template<typename T>
class UpdatableCollection : public IUpdatableCollection {
public:
	typedef typename SwissArray<T>::iterator iterator;

	UpdatableCollection() {
	}

	UpdatableCollection(uint32_t reserve) : m_items(reserve) {
	}

	virtual void update(HgTime dtime) final {
		if (empty()) return;
		if (dtime.msec() == 0) return;

		//new instances could be added after this update, so keep track of how many
		//instances were updated so that we don't render newly added but not updated
		//instances that have been created.
		m_updatedItems.clear();
		m_updatedItems.reserve(m_items.count());

		for (auto itr = begin(); itr != end(); itr++) {
			auto& i = *itr;
			i.T::update(dtime); //avoid vtable lookup
			m_updatedItems.push_back(&i);
		}

		//This type only handles running update function.
		//A different type handles adding items to the render queue
	}

	virtual void EnqueueForRender(RenderQueue* queue, HgTime dt) final {
		if (empty()) return;

		for (auto itr = begin(); itr != end(); itr++) {
			auto& e = itr->getEntity();
			if (!e.getFlags().hidden) {
				queue->Enqueue(&e, dt);
			}
		}
	}

	inline T& newItem() {
		return m_items.newItem();
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

	//returns a list of items that were updated during the last call to update
	inline auto& getUpdatedItems() const { return m_updatedItems; }

	static UpdatableCollection<T>& Collection() {
		static UpdatableCollection<T> collection;
		return collection;
	}
private:
	SwissArray<T> m_items;
	std::vector<T*> m_updatedItems; //has been updated and ready for render
};