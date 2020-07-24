#include <gtest/gtest.h>
#include <core/EntityIdType.h>

TEST(NoEntites, TestId0) {
	EntityIdType id;
	EXPECT_FALSE(EntityIdTable::Manager.exists(id));

	auto newId = EntityIdTable::Manager.create();
	EXPECT_FALSE(EntityIdTable::Manager.exists(id));
	EXPECT_TRUE(EntityIdTable::Manager.exists(newId));

	EntityIdTable::Manager.destroy(newId);
	EXPECT_FALSE(EntityIdTable::Manager.exists(id));
}

TEST(NoEntites, TestId1) {
	auto id = EntityIdTable::Manager.create();
	EXPECT_TRUE(EntityIdTable::Manager.exists(id));
	EntityIdTable::Manager.destroy(id);
	EXPECT_FALSE(EntityIdTable::Manager.exists(id));
}

TEST(NoEntites, TestIdMultiple) {
	auto id1 = EntityIdTable::Manager.create();
	auto id2 = EntityIdTable::Manager.create();
	auto id3 = EntityIdTable::Manager.create();
	auto id4 = EntityIdTable::Manager.create();

	EXPECT_TRUE(EntityIdTable::Manager.exists(id3));
	EntityIdTable::Manager.destroy(id3);
	EXPECT_FALSE(EntityIdTable::Manager.exists(id3));
	EXPECT_TRUE(EntityIdTable::Manager.exists(id2));
	EXPECT_TRUE(EntityIdTable::Manager.exists(id4));
}
