#pragma once

#include <stdint.h>
#include <HgTimer.h>

#include <HgElement.h>
//class HgEntity;

class IUpdatable {
public:
	virtual void update(HgTime tdelta) = 0;

	//inline void setElement(const HgEntity* x) { element = x; }
	inline HgEntity& getEntity() { return m_entity; }
	inline const HgEntity& getEntity() const { return m_entity; }

protected:
	//HgEntity* element; //just a weak pointer
	HgEntity m_entity;
};