#include <gtest/gtest.h>
#include <core/EntityIdType.h>

TEST(NoEntites, TestId0) {
	EntityIdType id;
	EXPECT_FALSE(EntityIdTable::Manager.exists(id));

	auto newId = EntityIdTable::Manager.create();
	EXPECT_FALSE(EntityIdTable::Manager.exists(id));
}

//TEST(AngleTest, Deg360) {
//	auto x = angle::deg(360);
//	EXPECT_NEAR(x.rad(), 6.28318530717958647, 0.000001);
//}