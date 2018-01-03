#pragma once

#include <quaternion.h>
#include <HgTypes.h>
#include <HgMath.h>

class Player {
private:
	point m_position;
	quaternion m_rotation;

	vector3 m_direction;


public:
	float speedMsec;
	vector3 m_moveDirection;

	void setPosition(point p) { m_position = p; }
	inline point position() const { return m_position; }

	void setRotation(quaternion q) { m_rotation = q; computeDirection(); }
	inline quaternion rotation() const { return m_rotation; }

	inline vector3 direction() const { return m_direction; }

private:
	void computeDirection() {
		vector3 ray = { 0, 0, -1 }; //correct.
		m_direction = vector3_normalize(vector3_quat_rotate(ray, m_rotation));
	}

};

extern Player PLAYER1;