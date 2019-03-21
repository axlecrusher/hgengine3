#pragma once

#include <HgTypes.h>

namespace HgSound {

	class Emitter
	{
	public:
		Emitter();
		~Emitter();

		void setPosition(const vector3f& p) { m_position = p; }
		const vector3f& getPosition() { return m_position; }

		void setVelocity(const velocity& v) { m_velocity = v; }
		const velocity& getVelocity() { return m_velocity; }

	private:
		vector3f m_position;
		velocity m_velocity;
	};

}