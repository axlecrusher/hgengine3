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


		inline vector3f getWorldSpacePosition() const { return position; }
		inline void setWorldSpacePosition(const point& pos) { position = pos; }

		//world space orientation
		inline void setWorldSpaceOrientation(const quaternion& r) { orientation = r; }
		inline quaternion getWorldSpaceOrientation() const { return orientation; }

		// Project a ray in the direction of the camera's view
		vector3 getForward() const;

		//compute world space vector the represents up
		vector3 getUp() const;

		HgMath::mat4f toViewMatrix() const;

	private:

		point position;
		quaternion orientation;

//		vector3 direction;
		float speedMsec;

private:

	vector3 initialDirection; //direction vector that rotations are relative to
};


vector3f ComputeStereoCameraPosition(const HgCamera& camera, float eyeOffsetFromCenter);