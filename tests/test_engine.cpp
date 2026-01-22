#include <gtest/gtest.h>
#include "../src/engine/DatabaseManager.h"

TEST(DatabaseTests, ConnectionCheck) {
    DatabaseManager db;
    EXPECT_EQ(db.testConnection(), "{\"status\":\"connected\"}");
}