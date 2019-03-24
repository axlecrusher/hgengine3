#pragma once

#include <quaternion.h>
#include <HgTypes.h>
#include <HgMath.h>

class Player {
private:
	HgEntity m_element;
	//SpacialData m_spacial;
	vector3 m_direction;
public:
	float speedMsec;
	vector3 m_moveDirection;

	void setPosition(const point& p) { m_element.position(p); }
	inline point position() const { return m_element.position(); }

	void setOrientation(const quaternion& q) { m_element.orientation(q); computeDirection(); }
	inline quaternion orientation() const { return m_element.orientation(); }

	inline vector3 direction() const { return m_direction; }

	inline const HgEntity& element() const { return m_element; }
	inline HgEntity& element() { return m_element; }

private:
	void computeDirection() {
		vector3 ray = { 0, 0, -1 }; //correct.
		m_direction = ray.rotate(orientation()).normal();
	}

};

extern Player PLAYER1;