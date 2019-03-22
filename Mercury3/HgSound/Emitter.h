#pragma once

#include <HgTypes.h>
#include <quaternion.h>

namespace HgSound {

	class Emitter
	{
	public:
		void setPosition(const vector3f& p) { m_position = p; }
		const vector3f& getPosition() const { return m_position; }

		void setVelocity(const velocity& v) { m_velocity = v; }
		const velocity& getVelocity() const { return m_velocity; }

		void setForward(const vector3f& forward) { m_forward = forward; }
		const vector3f& getForward() const { return m_forward; }

		void setUp(const vector3f& up) { m_up = up; }
		const vector3f& getUp() const { return m_up; }

	private:
		vector3f m_position;
		velocity m_velocity;
		vector3f m_forward;
		vector3f m_up;
	};

	class Listener
	{
	public:
		void setPosition(const vector3f& p) { m_position = p; }
		const vector3f& getPosition() const { return m_position; }

		void setVelocity(const velocity& v) { m_velocity = v; }
		const velocity& getVelocity() const { return m_velocity; }

		void setForward(const vector3f& forward) { m_forward = forward; }
		const vector3f& getForward() const { return m_forward; }

		void setUp(const vector3f& up) { m_up = up; }
		const vector3f& getUp() const { return m_up; }

	private:
		vector3f m_position;
		velocity m_velocity;
		vector3f m_forward;
		vector3f m_up;
	};

}