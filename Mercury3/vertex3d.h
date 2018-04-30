#pragma once

#include <cmath>
#include <memory>
#include <HgMath.h>
#include <quaternion.h>

class vertex3d {
public:
	vertex3d() : xyz{ 0,0,0 }
	{
	}

	vertex3d(float x, float y, float z) : xyz{ x,y,z }
	{
	}

	inline vertex3d(const float *_xyz)
	{
		xyz[0] = _xyz[0];
		xyz[1] = _xyz[1];
		xyz[2] = _xyz[2];
	}

	inline float x() const { return xyz[0]; }
	inline void x(float n) { xyz[0] = n; }

	inline float y() const { return xyz[1]; }
	inline void y(float n) { xyz[1] = n; }

	inline float z() const { return xyz[2]; }
	inline void z(float n) { xyz[2] = n; }

	inline vertex3d scale(float n) const {
		vertex3d tmp(*this);
		tmp.xyz[0] *= n;
		tmp.xyz[1] *= n;
		tmp.xyz[2] *= n;
		return tmp;
	}

	inline vertex3d operator+(const vertex3d& rhs) const {
		vertex3d tmp(*this);
		tmp.xyz[0] += rhs.xyz[0];
		tmp.xyz[1] += rhs.xyz[1];
		tmp.xyz[2] += rhs.xyz[2];
		return tmp;
	}

	inline vertex3d operator-(const vertex3d& rhs) const {
		vertex3d tmp(*this);
		tmp.xyz[0] -= rhs.xyz[0];
		tmp.xyz[1] -= rhs.xyz[1];
		tmp.xyz[2] -= rhs.xyz[2];
		return tmp;
	}

	inline vertex3d operator*(const vertex3d& rhs) const {
		vertex3d tmp(*this);
		tmp.xyz[0] *= rhs.xyz[0];
		tmp.xyz[1] *= rhs.xyz[1];
		tmp.xyz[2] *= rhs.xyz[2];
		return tmp;
	}

	inline vertex3d operator/(const vertex3d& rhs) const {
		vertex3d tmp(*this);
		tmp.xyz[0] /= rhs.xyz[0];
		tmp.xyz[1] /= rhs.xyz[1];
		tmp.xyz[2] /= rhs.xyz[2];
		return tmp;
	}

	inline float dot(const vertex3d& rhs) const {
		float r = xyz[0] * rhs.xyz[0] +
			xyz[1] * rhs.xyz[1] +
			xyz[2] * rhs.xyz[2];
		return r;
	}

	inline float length() const {
		float x = HgMath::square(xyz[0])
			+ HgMath::square(xyz[1])
			+ HgMath::square(xyz[2]);

		return (float)sqrt(x);
	}

	inline vertex3d normal() const {
		vertex3d tmp(*this);

		float length = this->length();
		//	if (fabs(length) < 0.000000000001f) return *v;
		if (length == 0.0) return tmp;

		tmp.xyz[0] /= length;
		tmp.xyz[1] /= length;
		tmp.xyz[2] /= length;

		return tmp;
	}

	inline vertex3d cross(const vertex3d& rhs) const {
		vertex3d r;
		r.x( (y() * rhs.z()) - (z() * rhs.y()) );
		r.y( (z() * rhs.x()) - (x() * rhs.z()) );
		r.z( (x() * rhs.y()) - (y() * rhs.x()) );
		return r;
	}

	inline vertex3d rotate(const quaternion& q) const {
		vertex3d r1 = this->scale(q.w());
		vertex3d r2 = vertex3d(q.wxyz.wxyz + 1).cross(*this); 
		vertex3d tmp = vertex3d(q.wxyz.wxyz + 1).cross(r1 + r2).scale(2.0); //XXX fix this
		return *this + tmp;
	}

	//inline bool operator<(const vertex3d& rhs) const {
	//	return ((x() < rhs.x())
	//		&& (y() < rhs.y())
	//		&& (z() < rhs.z()));
	//}
private:
	float xyz[3];
};