#pragma once

#include <vector>
#include <set>

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
		if (entityCount() == 0) return;

		for (T& i : m_entities) {
			//if ((dtime > 0) && i.needsUpdate(updateNumber)) {
			//	e.update(dtime, updateNumber);
			//}
			if (dtime > 0) {
				i.T::update(dtime); //avoid vtable lookup
			}
		}

		doRemovals();
	}

	inline T& newItem() {
		m_entities.push_back(T());
		return m_entities.back();
	}

	inline void remove(const T& x) {
		m_remove.insert(&x);
	}

	inline uint32_t entityCount() const { return m_entities.size(); }

	inline const std::vector<T>& entities() const { return m_entities; }
	inline std::vector<T>& entities() { return m_entities; }
private:

	void doRemovals() {
		if (m_remove.empty()) return;
		size_t i = m_entities.size() - 1;
		while (!m_entities.empty()) {
			const T* p = &(m_entities[i]);
			auto r = m_remove.find(p);
			if (r != m_remove.end()) m_entities.erase(m_entities.begin()+i);
			if (i == 0) break;
			i--;
		}
		m_remove.clear();
	}

	std::vector<T> m_entities;
	std::set<const T*> m_remove;
};