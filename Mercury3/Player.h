#pragma once

#include <quaternion.h>
#include <HgTypes.h>
#include <HgMath.h>

class Player {
private:
	point m_position;
	quaternion m_orientation;

	vector3 m_direction;


public:
	float speedMsec;
	vector3 m_moveDirection;

	void setPosition(const point& p) { m_position = p; }
	inline point position() const { return m_position; }

	void setOrientation(const quaternion& q) { m_orientation = q; computeDirection(); }
	inline quaternion orientation() const { return m_orientation; }

	inline vector3 direction() const { return m_direction; }

private:
	void computeDirection() {
		vector3 ray = { 0, 0, -1 }; //correct.
		m_direction = ray.rotate(m_orientation).normal();
	}

};

extern Player PLAYER1;