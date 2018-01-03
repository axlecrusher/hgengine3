#pragma once

#include <HgTypes.h>
#include <quaternion.h>

class HgCamera {
	public:
		point position;
		quaternion rotation;

		vector3 direction;
		float speedMsec;

		// Project a ray in the direction of the camera's view
		vector3 projectRay();
};
