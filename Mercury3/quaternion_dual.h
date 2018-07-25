#pragma once

#include <quaternion.h>

//Based on, though the implementation seemed buggy.
//A Beginners Guide to Dual-Quaternions
//Ben Kenwright
//http://wscg.zcu.cz/wscg2012/short/A29-full.pdf
//computations checked against https://www.euclideanspace.com/maths/algebra/realNormedAlgebra/other/dualQuaternion/calculator/index.htm

class dual_quaternion {
public:
	dual_quaternion() : m_dual(0, 0, 0, 0) {}
	dual_quaternion(const quaternion& rotation, const vector3& translation);

	inline dual_quaternion scale(float m) const;

	inline float dot(const dual_quaternion& rhs) const {
		return m_real.dot(rhs.m_real);
	}

	inline dual_quaternion dual_quaternion::normal() const {
		const float scaling = 1.0 / m_real.magnitude();
		return this->scale(scaling);
	}

	dual_quaternion operator+(const dual_quaternion& rhs) const;

	dual_quaternion operator*(const dual_quaternion& rhs) const;

	void read(float* r) const;

private:
	quaternion m_real;
	quaternion m_dual;
};