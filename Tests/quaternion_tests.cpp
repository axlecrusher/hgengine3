#include "gtest/gtest.h"
#include <quaternion.h>
#include <quaternion_dual.h>

#include <math/matrix.h>

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
		auto r = quaternion::mult(a, b);

		EXPECT_NEAR(r.w(), -60, 0.000001);
		EXPECT_NEAR(r.x(), 12, 0.000001);
		EXPECT_NEAR(r.y(), 30, 0.000001);
		EXPECT_NEAR(r.z(), 24, 0.000001);
	}

	TEST(QuaternionTest, VectorizedMultiply) {
		quaternion a(1, 2, 3, 4);
		quaternion b(9, 8, 7, 6);
		quaternion correct = quaternion::mult_c(a, b);
		quaternion r = quaternion::mult_vectorized(a, b);

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

	TEST(QuaternionTest, toMatrix) {
		using namespace HgMath;
		const vector3 axis(12, 3, 31);
		const auto rotAngle = angle::deg(20);
		const quaternion q = quaternion::fromAxisAngle(vector3(1.0, 2.0, 1.0), rotAngle);
		const mat4f cmatrix = mat4f::axisRotation(rotAngle.rad(), vectorial::vec3f(1.0, 2.0, 1.0));
		const auto r = q.toMatrix4();

		float matrix[16];
		r.store(matrix);

		EXPECT_NEAR(matrix[0], cmatrix.value.x.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[1], cmatrix.value.x.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[2], cmatrix.value.x.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[3], cmatrix.value.x.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[4], cmatrix.value.y.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[5], cmatrix.value.y.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[6], cmatrix.value.y.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[7], cmatrix.value.y.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[8], cmatrix.value.z.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[9], cmatrix.value.z.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[10], cmatrix.value.z.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[11], cmatrix.value.z.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[12], cmatrix.value.w.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[13], cmatrix.value.w.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[14], cmatrix.value.w.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[15], cmatrix.value.w.m128_f32[3], 0.000001);
	}

	TEST(DualQuaternionTest, Multiplication) {
		using namespace HgMath;
		const dual_quaternion dq(quaternion::fromEuler(angle::deg(15), angle::deg(20), angle::deg(25)), vector3(5, 10, 15));
		const dual_quaternion dq2(quaternion::fromEuler(angle::deg(25), angle::deg(30), angle::deg(35)), vector3(5, 15, 20));
		const auto r = dq * dq2;

		EXPECT_NEAR(r.w(), 0.6890499944848328, 0.000001);
		EXPECT_NEAR(r.x(), 0.39238278751707895, 0.000001);
		EXPECT_NEAR(r.y(), 0.47569033124701493, 0.000001);
		EXPECT_NEAR(r.z(), 0.3807420372101426, 0.000001);
		EXPECT_NEAR(r.dw(), -14.503239415857994, 0.000001);
		EXPECT_NEAR(r.dx(), -0.14249841758993775, 0.000001);
		EXPECT_NEAR(r.dy(), 11.692152175064496, 0.000001);
		EXPECT_NEAR(r.dz(), 11.786267675270569, 0.000001);
	}

	TEST(DualQuaternionTest, Addition) {
		using namespace HgMath;
		const dual_quaternion dq(quaternion::fromEuler(angle::deg(15), angle::deg(20), angle::deg(25)), vector3(5, 10, 15));
		const dual_quaternion dq2(quaternion::fromEuler(angle::deg(25), angle::deg(30), angle::deg(35)), vector3(5, 15, 20));
		const auto r = dq + dq2;

		EXPECT_NEAR(r.w(), 1.830870747, 0.000001);
		EXPECT_NEAR(r.x(), 0.43813102, 0.000001);
		EXPECT_NEAR(r.y(), 0.49975945099999997, 0.000001);
		EXPECT_NEAR(r.z(), 0.419347971, 0.000001);
		EXPECT_NEAR(r.dw(), -8.07424545, 0.000001);
		EXPECT_NEAR(r.dx(), 2.741452093, 0.000001);
		EXPECT_NEAR(r.dy(), 14.28674269, 0.000001);
		EXPECT_NEAR(r.dz(), 14.30818891, 0.000001);
	}

	TEST(DualQuaternionTest, Subtraction) {
		using namespace HgMath;
		const dual_quaternion dq(quaternion::fromEuler(angle::deg(15), angle::deg(20), angle::deg(25)), vector3(5, 10, 15));
		const dual_quaternion dq2(quaternion::fromEuler(angle::deg(25), angle::deg(30), angle::deg(35)), vector3(5, 15, 20));
		const auto r = dq - dq2;

		EXPECT_NEAR(r.w(), 0.06579458699999996, 0.000001);
		EXPECT_NEAR(r.x(), -0.11261297799999997, 0.000001);
		EXPECT_NEAR(r.y(), -0.10795246099999997, 0.000001);
		EXPECT_NEAR(r.z(), -0.04094895700000001, 0.000001);
		EXPECT_NEAR(r.dw(), 2.46342277, 0.000001);
		EXPECT_NEAR(r.dx(), 0.953653927, 0.000001);
		EXPECT_NEAR(r.dy(), -3.308027750000001, 0.000001);
		EXPECT_NEAR(r.dz(), -0.7312712700000006, 0.000001);
	}

	TEST(DualQuaternionTest, GetTranslation) {
		using namespace HgMath;
		const dual_quaternion dq(quaternion::fromEuler(angle::deg(0), angle::deg(0), angle::deg(0)), vector3(5, 10, 15));
		//const dual_quaternion dq2(quaternion::fromEuler(angle::deg(25), angle::deg(30), angle::deg(35)), vector3(5, 15, 20));
		//const auto r = dq * dq2;

		const auto r = dq.getTranslation();

		EXPECT_NEAR(r.x(), 5, 0.000001);
		EXPECT_NEAR(r.y(), 10, 0.000001);
		EXPECT_NEAR(r.z(), 15, 0.000001);
	}

	TEST(DualQuaternionTest, toMatrix_Xrot) {
		using namespace HgMath;
		const auto rotAngle = angle::deg(20);
		const dual_quaternion dq(quaternion::fromEuler(rotAngle, angle::deg(0), angle::deg(0)), vector3(5, 10, 15));
		mat4f cmatrix = mat4f::translation(vectorial::vec3f(5, 10, 15)) * mat4f::axisRotation(rotAngle.rad(), vectorial::vec3f(1.0, 0, 0));

		float matrix[16];
		dq.toMatrix4(matrix);

		EXPECT_NEAR(matrix[0], cmatrix.value.x.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[1], cmatrix.value.x.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[2], cmatrix.value.x.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[3], cmatrix.value.x.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[4], cmatrix.value.y.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[5], cmatrix.value.y.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[6], cmatrix.value.y.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[7], cmatrix.value.y.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[8], cmatrix.value.z.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[9], cmatrix.value.z.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[10], cmatrix.value.z.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[11], cmatrix.value.z.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[12], cmatrix.value.w.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[13], cmatrix.value.w.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[14], cmatrix.value.w.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[15], cmatrix.value.w.m128_f32[3], 0.000001);
	}

	TEST(DualQuaternionTest, toMatrix_Yrot) {
		using namespace HgMath;
		const auto rotAngle = angle::deg(20);
		const dual_quaternion dq(quaternion::fromEuler(angle::deg(0), rotAngle, angle::deg(0)), vector3(5, 10, 15));
		mat4f cmatrix = mat4f::translation(vectorial::vec3f(5, 10, 15)) * mat4f::axisRotation(rotAngle.rad(), vectorial::vec3f(0, 1.0, 0));

		float matrix[16];
		dq.toMatrix4(matrix);

		EXPECT_NEAR(matrix[0], cmatrix.value.x.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[1], cmatrix.value.x.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[2], cmatrix.value.x.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[3], cmatrix.value.x.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[4], cmatrix.value.y.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[5], cmatrix.value.y.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[6], cmatrix.value.y.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[7], cmatrix.value.y.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[8], cmatrix.value.z.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[9], cmatrix.value.z.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[10], cmatrix.value.z.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[11], cmatrix.value.z.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[12], cmatrix.value.w.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[13], cmatrix.value.w.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[14], cmatrix.value.w.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[15], cmatrix.value.w.m128_f32[3], 0.000001);
	}

	TEST(DualQuaternionTest, toMatrix_Zrot) {
		using namespace HgMath;
		const auto rotAngle = angle::deg(20);
		const dual_quaternion dq(quaternion::fromEuler(angle::deg(0), angle::deg(0), rotAngle), vector3(5, 10, 15));
		mat4f cmatrix = mat4f::translation(vectorial::vec3f(5, 10, 15)) * mat4f::axisRotation(rotAngle.rad(), vectorial::vec3f(0, 0, 1.0));

		float matrix[16];
		dq.toMatrix4(matrix);

		EXPECT_NEAR(matrix[0], cmatrix.value.x.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[1], cmatrix.value.x.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[2], cmatrix.value.x.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[3], cmatrix.value.x.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[4], cmatrix.value.y.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[5], cmatrix.value.y.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[6], cmatrix.value.y.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[7], cmatrix.value.y.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[8], cmatrix.value.z.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[9], cmatrix.value.z.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[10], cmatrix.value.z.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[11], cmatrix.value.z.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[12], cmatrix.value.w.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[13], cmatrix.value.w.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[14], cmatrix.value.w.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[15], cmatrix.value.w.m128_f32[3], 0.000001);
	}

	TEST(DualQuaternionTest, toMatrix_fromAxisAngle) {
		using namespace HgMath;
		const auto rotAngle = angle::deg(20);
		const dual_quaternion dq(quaternion::fromAxisAngle(vector3(1.0, 2.0, 1.0), rotAngle), vector3(5, 10, 15));
		mat4f cmatrix = mat4f::translation(vectorial::vec3f(5, 10, 15)) * mat4f::axisRotation(rotAngle.rad(), vectorial::vec3f(1.0, 2.0, 1.0));

		float matrix[16];
		dq.toMatrix4(matrix);

		EXPECT_NEAR(matrix[0], cmatrix.value.x.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[1], cmatrix.value.x.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[2], cmatrix.value.x.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[3], cmatrix.value.x.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[4], cmatrix.value.y.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[5], cmatrix.value.y.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[6], cmatrix.value.y.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[7], cmatrix.value.y.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[8], cmatrix.value.z.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[9], cmatrix.value.z.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[10], cmatrix.value.z.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[11], cmatrix.value.z.m128_f32[3], 0.000001);

		EXPECT_NEAR(matrix[12], cmatrix.value.w.m128_f32[0], 0.000001);
		EXPECT_NEAR(matrix[13], cmatrix.value.w.m128_f32[1], 0.000001);
		EXPECT_NEAR(matrix[14], cmatrix.value.w.m128_f32[2], 0.000001);
		EXPECT_NEAR(matrix[15], cmatrix.value.w.m128_f32[3], 0.000001);
	}
}