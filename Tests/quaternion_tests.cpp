#include "gtest/gtest.h"
#include <quaternion.h>

namespace {

	TEST(QuaternionTest, Conjugate) {
		quaternion a(1, 2, 3, 4);
		auto r = a.conjugate();
		EXPECT_NEAR(r.w(), 1.0, 0.000001);
		EXPECT_NEAR(r.x(), -2.0, 0.000001);
		EXPECT_NEAR(r.y(), -3.0, 0.000001);
		EXPECT_NEAR(r.z(), -4.0, 0.000001);
	}

	TEST(QuaternionTest, Inverse) {
		quaternion a(1, 2, 3, 4);
		auto r = a.inverse();
		EXPECT_NEAR(r.w(), 0.18257418274879456, 0.000001);
		EXPECT_NEAR(r.x(), -0.36514836549758911, 0.000001);
		EXPECT_NEAR(r.y(), -0.54772257804870605, 0.000001);
		EXPECT_NEAR(r.z(), -0.73029673099517822, 0.000001);
	}

	TEST(QuaternionTest, Magnitude) {
		quaternion a(1, 2, 3, 4);
		float r = a.magnitude();
		EXPECT_NEAR(r, 5.477225575051661, 0.000001);
	}

	TEST(QuaternionTest, Normal) {
		quaternion a(1, 2, 3, 4);
		quaternion b = a.normal();

		EXPECT_NEAR(b.magnitude(), 1.0, 0.000001);
		EXPECT_NEAR(b.squaredLength(), 1.0, 0.000001);
		EXPECT_NEAR(b.w(), 0.18257418274879456, 0.000001);
		EXPECT_NEAR(b.x(), 0.36514836549758911, 0.000001);
		EXPECT_NEAR(b.y(), 0.54772257804870605, 0.000001);
		EXPECT_NEAR(b.z(), 0.73029673099517822, 0.000001);
	}

	TEST(QuaternionTest, DotProduct) {
		quaternion a(2, 3, 4, 5);
		float r = a.dot(a);
		EXPECT_NEAR(r, 54.0, 0.000001);
	}

	TEST(QuaternionTest, Addition) {
		quaternion a(1, 2, 3, 4);
		quaternion b(4, 2, 6, 9);
		auto r = a + b;

		EXPECT_NEAR(r.w(), 5.0, 0.000001);
		EXPECT_NEAR(r.x(), 4.0, 0.000001);
		EXPECT_NEAR(r.y(), 9.0, 0.000001);
		EXPECT_NEAR(r.z(), 13.0, 0.000001);
	}

	TEST(QuaternionTest, Multiply) {
		quaternion a(1, 2, 3, 4);
		quaternion b(5, 6, 7, 8);
		auto r = quaternion::quat_mult(a, b);

		EXPECT_NEAR(r.w(), -60, 0.000001);
		EXPECT_NEAR(r.x(), 12, 0.000001);
		EXPECT_NEAR(r.y(), 30, 0.000001);
		EXPECT_NEAR(r.z(), 24, 0.000001);
	}

	TEST(QuaternionTest, VectorizedMultiply) {
		quaternion a(1, 2, 3, 4);
		quaternion b(9, 8, 7, 6);
		quaternion correct = quaternion::quat_mult(a, b);
		quaternion r = quaternion::quat_mult_vectorized(a, b);

		EXPECT_NEAR(r.w(), correct.w(), 0.000001);
		EXPECT_NEAR(r.x(), correct.x(), 0.000001);
		EXPECT_NEAR(r.y(), correct.y(), 0.000001);
		EXPECT_NEAR(r.z(), correct.z(), 0.000001);
	}

	TEST(QuaternionTest, Scale) {
		quaternion a(2, 3, 4, 5);
		auto r = a.scale(2.0);
		EXPECT_NEAR(r.w(), 4.0, 0.000001);
		EXPECT_NEAR(r.x(), 6.0, 0.000001);
		EXPECT_NEAR(r.y(), 8.0, 0.000001);
		EXPECT_NEAR(r.z(), 10.0, 0.000001);
	}

	TEST(QuaternionTest, fromAxisAngle) {
		using namespace HgMath;
		vector3 axis(12, 3, 31);
		auto r = quaternion::fromAxisAngle(axis, angle::rad(0.959931));
		EXPECT_NEAR(r.w(), 0.8870109, 0.000001);
		EXPECT_NEAR(r.x(), 0.1660138, 0.000001);
		EXPECT_NEAR(r.y(), 0.0415035, 0.000001);
		EXPECT_NEAR(r.z(), 0.428869, 0.000001);
	}

	TEST(QuaternionTest, fromEuler) {
		using namespace HgMath;
		vector3 axis(12, 3, 31);
		auto r = quaternion::fromEuler(angle::deg(280), angle::deg(32), angle::deg(91));
		EXPECT_NEAR(r.w(), -0.642499, 0.000001);
		EXPECT_NEAR(r.x(), 0.2824796, 0.000001);
		EXPECT_NEAR(r.y(), 0.2927109, 0.000001);
		EXPECT_NEAR(r.z(), -0.6494001, 0.000001);
	}
}