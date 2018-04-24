#pragma once

#include <stdint.h>

class HgElement;

class IUpdatable {
public:
	virtual void update(uint32_t tdelta) = 0;

	inline void setElement(HgElement* x) { element = x; }
	inline HgElement& getElement() { return *element; }
	inline const HgElement& getElement() const { return *element; }

protected:
	HgElement* element; //just a weak pointer
};