#pragma once

#include <cmath>
#include <HgMath.h>

class quaternion;

namespace HgMath {
	template<typename T>
	class vertex {
	public:
		static const vertex UNIT_X;
		static const vertex UNIT_Y;
		static const vertex UNIT_Z;

		vertex() : xyz{ 0,0,0 }
		{
		}

		vertex(T x, T y, T z) : xyz{ x,y,z }
		{
		}

		inline vertex(const T *_xyz)
		{
			xyz[0] = _xyz[0];
			xyz[1] = _xyz[1];
			xyz[2] = _xyz[2];
		}

		inline T x() const { return xyz[0]; }
		inline void x(T n) { xyz[0] = n; }

		inline T y() const { return xyz[1]; }
		inline void y(T n) { xyz[1] = n; }

		inline T z() const { return xyz[2]; }
		inline void z(T n) { xyz[2] = n; }

		inline vertex scale(T n) const {
			vertex tmp(*this);
			tmp.xyz[0] *= n;
			tmp.xyz[1] *= n;
			tmp.xyz[2] *= n;
			return tmp;
		}

		inline vertex operator+(const vertex& rhs) const {
			vertex tmp(*this);
			tmp.xyz[0] += rhs.xyz[0];
			tmp.xyz[1] += rhs.xyz[1];
			tmp.xyz[2] += rhs.xyz[2];
			return tmp;
		}

		inline vertex operator+=(const vertex& rhs) {
			xyz[0] += rhs.xyz[0];
			xyz[1] += rhs.xyz[1];
			xyz[2] += rhs.xyz[2];
			return *this;
		}

		inline vertex operator-(const vertex& rhs) const {
			vertex tmp(*this);
			tmp.xyz[0] -= rhs.xyz[0];
			tmp.xyz[1] -= rhs.xyz[1];
			tmp.xyz[2] -= rhs.xyz[2];
			return tmp;
		}

		inline vertex operator*(const vertex& rhs) const {
			vertex tmp(*this);
			tmp.xyz[0] *= rhs.xyz[0];
			tmp.xyz[1] *= rhs.xyz[1];
			tmp.xyz[2] *= rhs.xyz[2];
			return tmp;
		}

		inline vertex operator/(const vertex& rhs) const {
			vertex tmp(*this);
			tmp.xyz[0] /= rhs.xyz[0];
			tmp.xyz[1] /= rhs.xyz[1];
			tmp.xyz[2] /= rhs.xyz[2];
			return tmp;
		}

		inline T dot(const vertex& rhs) const {
			T a = xyz[0] * rhs.xyz[0];
			T b = xyz[1] * rhs.xyz[1];
			T c = xyz[2] * rhs.xyz[2];
			return a + b + c;
		}

		inline T length() const {
			return (T)sqrt(squaredLength());
		}

		inline T squaredLength() const {
			T a = HgMath::square(xyz[0]);
			T b = HgMath::square(xyz[1]);
			T c = HgMath::square(xyz[2]);
			return a + b + c;
		}

		inline vertex normal() const {
			vertex tmp(*this);

			T length = this->length();
			//	if (fabs(length) < 0.000000000001f) return *v;
			if (length == 0.0) return tmp;

			tmp.xyz[0] /= length;
			tmp.xyz[1] /= length;
			tmp.xyz[2] /= length;

			return tmp;
		}

		inline vertex cross(const vertex& rhs) const {
			vertex r;
			r.x((y() * rhs.z()) - (z() * rhs.y()));
			r.y((z() * rhs.x()) - (x() * rhs.z()));
			r.z((x() * rhs.y()) - (y() * rhs.x()));
			return r;
		}

		inline vertex rotate(const quaternion& q) const {
			vertex r1 = this->scale(q.w());
			vertex r2 = vertex(q.wxyz.wxyz + 1).cross(*this);
			vertex tmp = vertex(q.wxyz.wxyz + 1).cross(r1 + r2).scale(2.0); //XXX fix this
			return *this + tmp;
		}

		inline bool isZeroLength() const
		{
			return (squaredLength() < HgMath::square(1e-06f)); //does squaredLength need to be double here if 1e-06 is double?
		}

		//inline bool operator<(const vertex& rhs) const {
		//	return ((x() < rhs.x())
		//		&& (y() < rhs.y())
		//		&& (z() < rhs.z()));
		//}
	private:
		T xyz[3];
	};
}

typedef HgMath::vertex<double> vertex3d;
typedef HgMath::vertex<float> vertex3f;