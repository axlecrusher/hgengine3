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

inline dual_quaternion dual_quaternion::normal() const {
	dual_quaternion ret = *this;
	const float sl = m_real.squaredLength();
	if ((sl-1.0f) > 0.000001f) {
		//const float scaling = 1.0f / m_real.magnitude();
		const float scaling = 1.0f / HgMath::sqrt(sl);
		ret = this->scale(scaling);
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

void dual_quaternion::toMatrix(float* m) const {
	using namespace HgMath;

	const auto q = this->normal();
	//const auto q = m_real.normal();
	const auto tr = getTranslation();

	const float qx = q.x();
	const float qy = q.y();
	const float qz = q.z();
	const float qw = q.w();

	m[0] = 1.0 - 2 * square(qy) - 2 * square(qz);
	m[1] = 2*qx*qy - 2*qz*qw;
	m[2] = 2*qx*qz + 2*qy*qw;
	m[3] = tr.x();

	m[4] = 2 * qx*qy + 2 * qz*qw;
	m[5] = 1 - 2 * square(qx) - 2 * square(qz);
	m[6] = 2 * qy*qz - 2 * qx*qw;
	m[7] = tr.y();

	m[8] = 2 * qx*qz - 2 * qy*qw;
	m[9] = 2 * qy*qz + 2 * qx*qw;
	m[10] = 1 - 2 * square(qx) - 2 * square(qy);
	m[11] = tr.z();

	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;
}
