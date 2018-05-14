#pragma once

#include <stdint.h>
#include <HgTimer.h>

#include <HgElement.h>
//class HgElement;

class IUpdatable {
public:
	virtual void update(HgTime tdelta) = 0;

	//inline void setElement(const HgElement* x) { element = x; }
	inline HgElement& getElement() { return element; }
	inline const HgElement& getElement() const { return element; }

protected:
	//HgElement* element; //just a weak pointer
	HgElement element;
};