#pragma once

#include <HgTypes.h>
#include <quaternion.h>
#include <HgMath.h>
#include <src/math/matrix.h>

class HgCamera {
	public:
		HgCamera();

		void FreeRotate(HgMath::angle yaw, HgMath::angle pitch);

		inline void move(const vector3& v) { position = position + v;	}
		inline void setOrientation(const quaternion& r) { orientation = r; }

		// Project a ray in the direction of the camera's view
		vector3 projectRay() const;

		point position;
		quaternion orientation;

		vector3 direction;
		float speedMsec;

		HgMath::mat4f toMatrix() const;

private:

	vector3 initialDirection; //direction vector that rotations are relative to
};
