#pragma once

#include <HgTypes.h>

class HgCamera {
	public:
		point position;
		quaternion rotation;

		vector3 direction;
		float speedMsec;

		vector3 projectRay();
};
