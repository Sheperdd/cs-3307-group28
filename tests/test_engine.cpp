#include <gtest/gtest.h>
#include "../src/engine/DatabaseManager.h"
#include <cstdio> // For removing the file

// This runs before EACH test to give us a clean slate
class DatabaseTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Delete the DB file so we start fresh every time
        std::remove("torquedesk.db");
    }
};

TEST_F(DatabaseTests, AddUserIncreasesCount) {
    DatabaseManager db;

    // Initial count should be 0
    EXPECT_EQ(db.getUserCount(), 0);

    // Add a user
    bool success = db.addUser("Kay", "kay@torquedesk.com", "123456");

    EXPECT_TRUE(success);
    EXPECT_EQ(db.getUserCount(), 1);
}