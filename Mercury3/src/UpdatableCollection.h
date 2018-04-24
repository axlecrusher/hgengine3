#pragma once

#include <vector>

class IUpdatableCollection {
public:
	virtual void update(uint32_t dtime) = 0;
};

/* This is for data oriented processing. Large collections of objects can
be stored contiguously in memory. Updates benefit from cache hits.*/
template<typename T>
class UpdatableCollection : public IUpdatableCollection {
public:
	UpdatableCollection() {}

	UpdatableCollection(uint32_t reserve) {
		m_entities.reserve(reserve);
	}

	virtual void update(uint32_t dtime) final {
		for (T& i : m_entities) {
			//if ((dtime > 0) && i.needsUpdate(updateNumber)) {
			//	e.update(dtime, updateNumber);
			//}
			if (dtime > 0) {
				i.T::update(dtime); //avoid vtable lookup
			}
		}
	}

	inline T& newItem() {
		m_entities.push_back(T());
		return m_entities.back();
	}

	inline const std::vector<T>& entities() const { return m_entities; }
	inline std::vector<T>& entities() { return m_entities; }
private:
	std::vector<T> m_entities;
};