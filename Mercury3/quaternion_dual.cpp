#include <quaternion_dual.h>

dual_quaternion::dual_quaternion(const quaternion& rotation, const vector3& translation)
	: m_real(rotation.normal())
{
	const auto tmp = quaternion(0, translation.x(), translation.y(), translation.z()) * m_real;
	m_dual = tmp.scale(0.5f);
}

dual_quaternion dual_quaternion::scale(float m) const {
	dual_quaternion r;
	r.m_real = m_real.scale(m);
	r.m_dual = m_dual.scale(m);
	return r;
}

dual_quaternion dual_quaternion::operator+(const dual_quaternion& rhs) const {
	dual_quaternion r;
	r.m_real = m_real + rhs.m_real;
	r.m_dual = m_dual + rhs.m_dual;
	return r;
}

dual_quaternion dual_quaternion::operator*(const dual_quaternion& rhs) const {
	//left to right order
	dual_quaternion r;
	r.m_real = m_real * rhs.m_real;
	r.m_dual = m_real * rhs.m_dual + m_dual * rhs.m_real; //this is exactly 2x. why?
	r.m_dual = r.m_dual.scale(0.5);
	return r;
}

void dual_quaternion::read(float* r) const {
	for (int i = 0; i < 4; i++) {
		r[i] = m_real.wxyz.wxyz[i];
		r[i + 4] = m_dual.wxyz.wxyz[i];
	}
}

