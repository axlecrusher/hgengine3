#pragma once

#include <quaternion.h>
#include <HgTypes.h>
#include <HgMath.h>

class Player {
private:
	HgEntity m_entity;
	//SpacialData m_spacial;
	vector3 m_direction;
public:
	float speedMsec;
	vector3 m_moveDirection;

	void setPosition(const point& p) { m_entity.position(p); }
	inline point position() const { return m_entity.position(); }

	void setOrientation(const quaternion& q) { m_entity.orientation(q); computeDirection(); }
	inline quaternion orientation() const { return m_entity.orientation(); }

	inline vector3 direction() const { return m_direction; }

	inline const HgEntity& entity() const { return m_entity; }
	inline HgEntity& entity() { return m_entity; }

private:
	void computeDirection() {
		vector3 ray = { 0, 0, -1 }; //correct.
		m_direction = ray.rotate(orientation()).normal();
	}

};

extern Player PLAYER1;