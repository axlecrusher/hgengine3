#pragma once

#include <quaternion.h>
#include <vertex3d.h>

#include <math/MatrixMath.h>

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

	dual_quaternion normal() const;

	dual_quaternion operator+(const dual_quaternion& rhs) const;

	dual_quaternion operator-(const dual_quaternion& rhs) const;

	dual_quaternion operator*(const dual_quaternion& rhs) const;

	vertex3f getTranslation() const;

	inline float w() const { return m_real.w(); }
	inline void w(float a) { m_real.w(a); }

	inline float x() const { return m_real.x(); }
	inline void x(float a) { m_real.x(a); }

	inline float y() const { return m_real.y(); }
	inline void y(float a) { m_real.y(a); }

	inline float z() const { return m_real.z(); }
	inline void z(float a) { m_real.z(a); }

	inline float dw() const { return m_dual.w(); }
	inline void dw(float a) { m_dual.w(a); }

	inline float dx() const { return m_dual.x(); }
	inline void dx(float a) { m_dual.x(a); }

	inline float dy() const { return m_dual.y(); }
	inline void dy(float a) { m_dual.y(a); }

	inline float dz() const { return m_dual.z(); }
	inline void dz(float a) { m_dual.z(a); }

	void toMatrix4(float* m) const;
	HgMath::mat4f toMatrix4() const;

private:
	quaternion m_real;
	quaternion m_dual;
};