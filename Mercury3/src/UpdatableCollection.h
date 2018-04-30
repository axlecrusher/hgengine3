#pragma once

#include <SwissArray.h>
#include <HgTimer.h>

#include <RenderBackend.h>

class IUpdatableCollection {
public:
	virtual void update(HgTime dtime) = 0;
};

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
		if (dtime.msec() >= 1) {
			updateAndRender(dtime);
		}
		else {
			renderOnly();
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

private:

	void updateAndRender(HgTime dtime) {
		for (auto itr = begin(); itr != end(); itr++) {
			itr->T::update(dtime); //avoid vtable lookup
			if (itr.itemValid()) //check for deletion
			{
				submitRender(itr->T::getElement());
			}
		}
	}

	void renderOnly() {
		for (auto itr = begin(); itr != end(); itr++) {
			submitRender(itr->T::getElement());
		}
	}

	inline void submitRender(HgElement& e) {
		if (!e.check_flag(HGE_HIDDEN)) {
			Renderer::Enqueue(e);
		}
	}

	SwissArray<T> m_items;
};