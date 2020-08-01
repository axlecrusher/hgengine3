#include <gtest/gtest.h>
#include <core/EntityIdType.h>

TEST(NoEntites, TestId0) {
	EntityIdType id;
	EXPECT_FALSE(EntityIdTable::Singleton().exists(id));

	auto newId = EntityIdTable::Singleton().create();
	EXPECT_FALSE(EntityIdTable::Singleton().exists(id));
	EXPECT_TRUE(EntityIdTable::Singleton().exists(newId));

	EntityIdTable::Singleton().destroy(newId);
	EXPECT_FALSE(EntityIdTable::Singleton().exists(id));
}

TEST(NoEntites, TestId1) {
	auto id = EntityIdTable::Singleton().create();
	EXPECT_TRUE(EntityIdTable::Singleton().exists(id));
	EntityIdTable::Singleton().destroy(id);
	EXPECT_FALSE(EntityIdTable::Singleton().exists(id));
}

TEST(NoEntites, TestIdMultiple) {
	auto id1 = EntityIdTable::Singleton().create();
	auto id2 = EntityIdTable::Singleton().create();
	auto id3 = EntityIdTable::Singleton().create();
	auto id4 = EntityIdTable::Singleton().create();

	EXPECT_TRUE(EntityIdTable::Singleton().exists(id3));
	EntityIdTable::Singleton().destroy(id3);
	EXPECT_FALSE(EntityIdTable::Singleton().exists(id3));
	EXPECT_TRUE(EntityIdTable::Singleton().exists(id2));
	EXPECT_TRUE(EntityIdTable::Singleton().exists(id4));
}
