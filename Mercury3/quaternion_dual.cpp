#include <quaternion_dual.h>

dual_quaternion::dual_quaternion(const quaternion& rotation, const vector3& translation)
	: m_real(rotation.normal())
{
	//result must not be normalized
	const auto tmp = quaternion::mult(quaternion(0, translation.x(), translation.y(), translation.z()), m_real);
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

dual_quaternion dual_quaternion::operator-(const dual_quaternion& rhs) const {
	dual_quaternion r;
	r.m_real = m_real - rhs.m_real;
	r.m_dual = m_dual - rhs.m_dual;
	return r;
}

dual_quaternion dual_quaternion::operator*(const dual_quaternion& rhs) const {
	//left to right order
	dual_quaternion r;
	r.m_real = m_real * rhs.m_real;
	//I don't know if this needs to be normalized or not
	r.m_dual = (m_real * rhs.m_dual + m_dual * rhs.m_real).scale(0.5); //this is exactly 2x. why?
	//r.m_dual = (quaternion::quat_mult(m_real, rhs.m_dual) + quaternion::quat_mult(rhs.m_dual, m_real));// .scale(0.5);
	return r;
}

HgMath::vec3f dual_quaternion::getTranslation() const {
	using namespace HgMath;
	vec3f r;

	//result must not be normalized
	quaternion tmp = quaternion::mult(m_dual.scale(2.0), m_real.conjugate());

	r.x(tmp.x());
	r.y(tmp.y());
	r.z(tmp.z());
	return r;
}
