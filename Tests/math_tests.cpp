#include <gtest/gtest.h>
#include <HgMath.h>

using namespace HgMath;

TEST(AngleTest, Deg0) {
	auto x = angle::deg(0);
	EXPECT_NEAR(x.rad(), 0.0f, 0.000001);
}

TEST(AngleTest, Deg360) {
	auto x = angle::deg(360);
	EXPECT_NEAR(x.rad(), 6.28318530717958647, 0.000001);
}