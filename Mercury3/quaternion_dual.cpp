#include <quaternion_dual.h>

dual_quaternion::dual_quaternion(const quaternion& rotation, const vector3& translation)
	: m_real(rotation.normal())
{
	//result must not be normalized
	m_dual = quaternion::mult(quaternion(0, translation.x(), translation.y(), translation.z()), m_real).scale(0.5f);
}

dual_quaternion dual_quaternion::scale(float m) const {
	dual_quaternion r;
	r.m_real = m_real.scale(m);
	r.m_dual = m_dual.scale(m);
	return r;
}

inline dual_quaternion dual_quaternion::normal() const {
	dual_quaternion ret = *this;
	const float length = m_real.magnitude();
	//if very close to unit length, don't compute. more stable
	if (std::abs(1.0f - length) > 1e-06f) {
		if (length > 1e-06f) {
			ret = this->scale(1.0f / length);
		}
	}
	return ret;
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
	r.m_real = quaternion::mult(m_real, rhs.m_real);
	//The equations below seems correct.
	r.m_dual = (quaternion::mult(m_real, rhs.m_dual) + quaternion::mult(m_dual, rhs.m_real));
	return r.normal();
}

vertex3f dual_quaternion::getTranslation() const {
	using namespace HgMath;
	vertex3f r;

	//result must not be normalized
	quaternion tmp = quaternion::mult(m_dual.scale(2.0), m_real.conjugate());

	r.x(tmp.x());
	r.y(tmp.y());
	r.z(tmp.z());
	return r;
}

void dual_quaternion::toMatrix4(float* m) const {
	const auto q = this->normal();
	//const auto q = m_real.normal();
	const auto tr = q.getTranslation();

	q.m_real.toMatrix4(m);
	
	m[12] = tr.x();
	m[13] = tr.y();
	m[14] = tr.z();
	m[15] = 1;
}

HgMath::mat4f dual_quaternion::toMatrix4() const {
	using namespace HgMath;

	float mm[16];
	toMatrix4(mm);

	return HgMath::mat4f(mm);
}