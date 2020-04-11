#pragma once

#include <stdint.h>
#include <HgTimer.h>

#include <HgEntity.h>
//class HgEntity;

class IUpdatable {
public:
	IUpdatable() = default;
	virtual ~IUpdatable() {}
	IUpdatable(IUpdatable&&) = default;

	virtual void update(HgTime tdelta) = 0;

	//inline void setEntity(const HgEntity* x) { element = x; }
	inline HgEntity& getEntity() { return m_entity; }
	inline const HgEntity& getEntity() const { return m_entity; }

	virtual void init() {
		m_entity.init();
	}

protected:
	//HgEntity* m_entity; //just a weak pointer
	HgEntity m_entity;
};