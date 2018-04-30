#pragma once

#include <stdint.h>
#include <HgTimer.h>

class HgElement;

class IUpdatable {
public:
	virtual void update(HgTime tdelta) = 0;

	inline void setElement(HgElement* x) { element = x; }
	inline HgElement& getElement() { return *element; }
	inline const HgElement& getElement() const { return *element; }

protected:
	HgElement* element; //just a weak pointer
};