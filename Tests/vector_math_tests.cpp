#include <gtest/gtest.h>
#include <math/vector.h>

typedef HgMath::vec4f vec4;

TEST(VectorMathTests, Scale) {
	vec4 a(1, 2, 3, 4);
	vec4 r = a.scale(2);

	EXPECT_NEAR(r[0], 2.0, 0.000001);
	EXPECT_NEAR(r[1], 4.0, 0.000001);
	EXPECT_NEAR(r[2], 6.0, 0.000001);
	EXPECT_NEAR(r[3], 8.0, 0.000001);
}

TEST(VectorMathTests, Addition) {
	vec4 a(1, 2, 3, 4);
	vec4 b(10, 20, 30, 40);
	vec4 r = a+b;

	EXPECT_NEAR(r[0], 11.0, 0.000001);
	EXPECT_NEAR(r[1], 22.0, 0.000001);
	EXPECT_NEAR(r[2], 33.0, 0.000001);
	EXPECT_NEAR(r[3], 44.0, 0.000001);
}

TEST(VectorMathTests, Subtraction) {
	vec4 a(1, 2, 3, 4);
	vec4 b(11, 22, 33, 44);
	vec4 r = b-a;

	EXPECT_NEAR(r[0], 10.0, 0.000001);
	EXPECT_NEAR(r[1], 20.0, 0.000001);
	EXPECT_NEAR(r[2], 30.0, 0.000001);
	EXPECT_NEAR(r[3], 40.0, 0.000001);
}

TEST(VectorMathTests, Multiplication) {
	vec4 a(1, 2, 3, 4);
	vec4 b(2, 3, 4, 5);
	vec4 r = a*b;

	EXPECT_NEAR(r[0], 2.0, 0.000001);
	EXPECT_NEAR(r[1], 6.0, 0.000001);
	EXPECT_NEAR(r[2], 12.0, 0.000001);
	EXPECT_NEAR(r[3], 20.0, 0.000001);
}

TEST(VectorMathTests, Division) {
	vec4 a(16, 32, 64, 128);
	vec4 b(16, 8, 4, 2);
	vec4 r = a/b;

	EXPECT_NEAR(r[0], 1.0, 0.000001);
	EXPECT_NEAR(r[1], 4.0, 0.000001);
	EXPECT_NEAR(r[2], 16.0, 0.000001);
	EXPECT_NEAR(r[3], 64.0, 0.000001);
}

TEST(VectorMathTests, SquaredLength) {
	vec4 a(2, 3, 4, 5);
	float r = a.squaredLength();
	EXPECT_NEAR(r, 54.0, 0.000001);
}

TEST(VectorMathTests, DotProduct) {
	vec4 a(2, 3, 4, 5);
	vec4 b(6, 7, 8, 9);
	float r = a.dot(b);
	EXPECT_NEAR(r, 110.0, 0.000001);
}

TEST(VectorMathTests, Magnitude) {
	vec4 a(2, 3, 4, 5);
	float r = a.magnitude();
	EXPECT_NEAR(r, 7.348469228349534, 0.000001);
}

TEST(VectorMathTests, Normal) {
	vec4 a(2, 3, 4, 5);
	float r = a.normal().magnitude();
	EXPECT_NEAR(r, 1.0, 0.000001);
}

TEST(VectorMathTests, Negate) {
	vec4 a(2, 3, 4, 5);
	auto r = a.negate();

	EXPECT_NEAR(r[0], -2.0, 0.000001);
	EXPECT_NEAR(r[1], -3.0, 0.000001);
	EXPECT_NEAR(r[2], -4.0, 0.000001);
	EXPECT_NEAR(r[3], -5.0, 0.000001);
}

TEST(VectorMathTests, ZeroLength) {
	vec4 a(2, 3, 4, 5);
	vec4 b(0, 0, 0, 0);

	EXPECT_FALSE(a.isZeroLength());
	EXPECT_TRUE(b.isZeroLength());
}