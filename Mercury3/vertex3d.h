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
		inline vertex x(T n) { xyz[0] = n; return *this; }
		inline vertex x(T n) const { auto tmp = *this; tmp.xyz[0] = n; return tmp; }

		inline T y() const { return xyz[1]; }
		inline vertex y(T n) { xyz[1] = n; return *this; }
		inline vertex y(T n) const { auto tmp = *this; tmp.xyz[1] = n; return tmp; }

		inline T z() const { return xyz[2]; }
		inline vertex z(T n) { xyz[2] = n; return *this; }
		inline vertex z(T n) const { auto tmp = *this; tmp.xyz[2] = n; return tmp; }

		inline vertex scale(T n) const {
			//Writing functions exactly like this lets the compiler make a lot of good optimizations.
			vertex tmp;
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] = xyz[i] * n;
			}
			return tmp;
		}

		inline vertex operator*(T rhs) const {
			return this->scale(rhs);
		}

		inline vertex operator+(const vertex& rhs) const {
			vertex tmp;
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] = xyz[i] + rhs.xyz[i];
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
			vertex tmp;
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] = xyz[i] - rhs.xyz[i];
			}
			return tmp;
		}

		inline vertex operator*(const vertex& rhs) const {
			vertex tmp;
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] = xyz[i] * rhs.xyz[i];
			}
			return tmp;
		}

		inline vertex& operator*=(const vertex& rhs) {
			auto tmp = *this * rhs;
			*this = tmp;
			//xyz = tmp.xyz;
			return *this;
		}

		inline vertex operator/(const vertex& rhs) const {
			vertex tmp;
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] = xyz[i] / rhs.xyz[i];
			}
			return tmp;
		}

		inline vertex operator/(const double rhs) const {
			vertex tmp;
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] = (T)(xyz[i] / rhs);
			}
			return tmp;
		}

		inline vertex operator*(const double rhs) const {
			vertex tmp;
			for (int i = 0; i < fcount; i++) {
				tmp.xyz[i] = (T)(xyz[i] * rhs);
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

		inline T magnitude() const {
			return HgMath::sqrt(squaredLength());
		}

		inline T squaredLength() const {
			T r = 0;
			for (int i = 0; i < fcount; i++) {
				r += HgMath::square(xyz[i]);
			}
			return r;
		}

		inline vertex normal() const {
			vertex r = *this;
			T length = magnitude();
			//if very close to unit length, don't compute. more stable
			if (std::abs(1.0f - length) > 1e-06f) {
				if (length > 1e-06f) {
					r = this->scale(T(1.0) / length);
				}
			}
			return r;
		}

		inline vertex cross(const vertex& rhs) const {
			vertex r;
			r.x((y() * rhs.z()) - (z() * rhs.y()));
			r.y((z() * rhs.x()) - (x() * rhs.z()));
			r.z((x() * rhs.y()) - (y() * rhs.x()));
			return r;
		}

		inline vertex rotate(const quaternion& q) const {
			vertex ret = *this;
			const vertex r1 = this->scale(q.w());
			const vertex r2 = vertex(q.raw() + 1).cross(*this);
			ret += vertex(q.raw() + 1).cross(r1 + r2).scale(2.0); //XXX fix this
			return ret;
		}

		inline bool isZeroLength() const
		{
			return (squaredLength() < HgMath::square(1e-06f)); //does squaredLength need to be double here if 1e-06 is double?
		}

		inline T* raw() { return xyz; }
		inline const T* raw() const { return xyz; }

		vertex center(const vertex& rhs) const {
			const auto v = *this - rhs;
			return rhs + v.scale(0.5);
		}

	private:
		T xyz[fcount];
	};

	//Project A onto B.
	template<typename T>
	inline vertex<T> Project(const vertex<T>& A, const vertex<T>& B)
	{
		return (A.dot(B) / B.squaredLength()) * B;
	}

	//Compute component of A that is perpendicular to B
	template<typename T>
	inline vertex<T> Reject(const vertex<T>& A, const vertex<T>& B)
	{
		const auto tmp = A.dot(B) / B.squaredLength();
		return A - (B * tmp);
	}
}

typedef HgMath::vertex<double> vertex3d;
typedef HgMath::vertex<float> vertex3f;
