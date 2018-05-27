#pragma once

#include <cmath>
#include <HgMath.h>
#include <cstring>

class quaternion;

namespace HgMath {
	template<typename T>
	class vertex {
		constexpr static int fcount = 3;
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

		inline vertex(const T _xyz[fcount])
		{
			for (int i = 0; i < fcount; i++) {
				xyz[i] = _xyz[i];
			}
		}

		inline T x() const { return xyz[0]; }
		inline void x(T n) { xyz[0] = n; }

		inline T y() const { return xyz[1]; }
		inline void y(T n) { xyz[1] = n; }

		inline T z() const { return xyz[2]; }
		inline void z(T n) { xyz[2] = n; }

		inline vertex scale(T n) const {
			vertex tmp(*this);
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] *= n;
			}
			return tmp;
		}

		inline vertex operator+(const vertex& rhs) const {
			vertex tmp(*this);
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] += rhs.xyz[i];
			}
			return tmp;
		}

		inline const vertex& operator+=(const vertex& rhs) {
			for (int i = 0; i < fcount; i++) {
				xyz[i] += rhs.xyz[i];
			}
			return *this;
		}

		inline vertex operator-(const vertex& rhs) const {
			vertex tmp(*this);
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] -= rhs.xyz[i];
			}
			return tmp;
		}

		inline vertex operator*(const vertex& rhs) const {
			vertex tmp(*this);
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] *= rhs.xyz[i];
			}
			return tmp;
		}

		inline vertex operator/(const vertex& rhs) const {
			vertex tmp(*this);
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] /= rhs.xyz[i];
			}
			return tmp;
		}

		inline T dot(const vertex& rhs) const {
			T r = 0;
			for (int i = 0; i < fcount; i++) {
				r += xyz[i] * rhs.xyz[i];
			}
			return r;
		}

		inline T length() const {
			return (T)sqrt(squaredLength());
		}

		inline T squaredLength() const {
			T r = 0;
			for (int i = 0; i < fcount; i++) {
				r += HgMath::square(xyz[i]);
			}
			return r;
		}

		inline vertex normal() const {
			vertex tmp(*this);

			T length = this->length();
			//	if (fabs(length) < 0.000000000001f) return *v;
			if (length == 0.0) return tmp;

			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] /= length;
			}

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
		T xyz[fcount];
	};
}

typedef HgMath::vertex<double> vertex3d;
typedef HgMath::vertex<float> vertex3f;