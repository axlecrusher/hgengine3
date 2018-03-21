#pragma once

#include <HgTypes.h>
#include <quaternion.h>
#include <HgMath.h>

class HgCamera {
	public:
		HgCamera();

		void FreeRotate(double yaw, double pitch);

		inline void Move(const vector3& v) { position = vector3_add(&position, &v);	}
		inline void SetRotation(const quaternion& r) { rotation = r; }

		// Project a ray in the direction of the camera's view
		vector3 projectRay();

		point position;
		quaternion rotation;

		vector3 direction;
		float speedMsec;

private:

	vector3 initialDirection; //direction vector that rotations are relative to
};
