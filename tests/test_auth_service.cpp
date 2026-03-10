/**
 * @file test_auth_service.cpp
 * @brief Unit tests for AuthService (GoogleTest).
 *
 * Covers: user registration and login.
 */
#include <gtest/gtest.h>
#include "../src/engine/DatabaseManager.h"
#include "../src/engine/AuthService.h"
#include "../src/engine/RatingEngine.h"
#include "../src/engine/ProfitabilityEngine.h"
#include <cstdio>

/// @name Minimal stub implementations
/// @brief Required for linking — the real engine logic is not under test here.
/// @{

RatingEngine::RatingEngine(DatabaseManager &db, int recencyHalfLifeDays)
    : db(db), recencyHalfLifeDays(recencyHalfLifeDays) {}

int RatingEngine::compute(MechanicID) { return 0; }

double RatingEngine::getechanicScore(MechanicID) { return 0.0; }

ProfitabilityEngine::ProfitabilityEngine(DatabaseManager &db, int defaiultHourlyRate, double taxRate)
    : db(db), defaultHourlyRate(defaiultHourlyRate), taxRate(taxRate) {}

void ProfitabilityEngine::seTaxRate(double newTaxRate) { taxRate = newTaxRate; }

void ProfitabilityEngine::setDefaultHourlyRate(int newRate) { defaultHourlyRate = newRate; }

/// @}

/**
 * @class AuthServiceTests
 * @brief GoogleTest fixture for AuthService.
 *
 * Deletes the SQLite database file before each test so every case
 * starts with a clean, empty schema.
 */
class AuthServiceTests : public ::testing::Test
{
protected:
  /// @brief Remove the DB file so every test begins with a blank database.
  void SetUp() override
  {
    std::remove("torquedesk.db");
  }
};

// =====================================================================
/// @name 1. registerUser Tests
/// @{
// =====================================================================

/// @brief Verify that registerUser returns correct fields for a CUSTOMER.
TEST_F(AuthServiceTests, RegisterUser_HappyPath_Customer)
{
  DatabaseManager db;
  AuthService auth(db);

  auto result = auth.registerUser("Alice Smith", "alice@test.com",
                                  "555-1234", "password123", UserRole::CUSTOMER);

  EXPECT_GT(result.userId, 0);
  EXPECT_EQ(result.fullName, "Alice Smith");
  EXPECT_EQ(result.email, "alice@test.com");
  EXPECT_EQ(result.role, UserRole::CUSTOMER);
}

/// @brief Verify that registerUser returns correct fields for a MECHANIC.
TEST_F(AuthServiceTests, RegisterUser_HappyPath_Mechanic)
{
  DatabaseManager db;
  AuthService auth(db);

  auto result = auth.registerUser("Bob Wrench", "bob@test.com",
                                  "555-5678", "securepass1", UserRole::MECHANIC);

  EXPECT_GT(result.userId, 0);
  EXPECT_EQ(result.fullName, "Bob Wrench");
  EXPECT_EQ(result.email, "bob@test.com");
  EXPECT_EQ(result.role, UserRole::MECHANIC);
}

/// @brief Verify that registerUser throws invalid_argument when name is empty.
TEST_F(AuthServiceTests, RegisterUser_EmptyName)
{
  DatabaseManager db;
  AuthService auth(db);

  EXPECT_THROW(auth.registerUser("", "a@test.com", "555", "password123",
                                 UserRole::CUSTOMER),
               std::invalid_argument);
}

/// @brief Verify that registerUser throws invalid_argument when email is empty.
TEST_F(AuthServiceTests, RegisterUser_EmptyEmail)
{
  DatabaseManager db;
  AuthService auth(db);

  EXPECT_THROW(auth.registerUser("Name", "", "555", "password123",
                                 UserRole::CUSTOMER),
               std::invalid_argument);
}

/// @brief Verify that registerUser throws invalid_argument when password is too short.
TEST_F(AuthServiceTests, RegisterUser_PasswordTooShort)
{
  DatabaseManager db;
  AuthService auth(db);

  EXPECT_THROW(auth.registerUser("Name", "short@test.com", "555", "1234567",
                                 UserRole::CUSTOMER),
               std::invalid_argument);
}

/// @brief Verify that registerUser throws invalid_argument for duplicate email.
TEST_F(AuthServiceTests, RegisterUser_DuplicateEmail)
{
  DatabaseManager db;
  AuthService auth(db);

  auth.registerUser("First", "dup@test.com", "555", "password123",
                    UserRole::CUSTOMER);

  EXPECT_THROW(auth.registerUser("Second", "dup@test.com", "555", "password456",
                                 UserRole::CUSTOMER),
               std::invalid_argument);
}

/// @brief Verify that a password with exactly 8 characters is accepted.
TEST_F(AuthServiceTests, RegisterUser_PasswordExactlyEightChars)
{
  DatabaseManager db;
  AuthService auth(db);

  auto result = auth.registerUser("Edge", "edge@test.com", "555", "12345678",
                                  UserRole::CUSTOMER);
  EXPECT_GT(result.userId, 0);
}

/// @}

// =====================================================================
/// @name 2. loginUser Tests
/// @{
// =====================================================================

/// @brief Verify that loginUser returns AuthenticatedUser with correct fields.
TEST_F(AuthServiceTests, LoginUser_HappyPath)
{
  DatabaseManager db;
  AuthService auth(db);

  auth.registerUser("Carol", "carol@test.com", "555", "password123",
                    UserRole::CUSTOMER);

  auto result = auth.loginUser("carol@test.com", "password123");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->fullName, "Carol");
  EXPECT_EQ(result->email, "carol@test.com");
  EXPECT_EQ(result->role, UserRole::CUSTOMER);
  EXPECT_GT(result->userId, 0);
}

/// @brief Verify that loginUser returns nullopt for a non-existent email.
TEST_F(AuthServiceTests, LoginUser_WrongEmail)
{
  DatabaseManager db;
  AuthService auth(db);

  auto result = auth.loginUser("nobody@test.com", "password123");
  EXPECT_FALSE(result.has_value());
}

/// @brief Verify that loginUser returns nullopt for an incorrect password.
TEST_F(AuthServiceTests, LoginUser_WrongPassword)
{
  DatabaseManager db;
  AuthService auth(db);

  auth.registerUser("Dave", "dave@test.com", "555", "correctpass1",
                    UserRole::MECHANIC);

  auto result = auth.loginUser("dave@test.com", "wrongpassword");
  EXPECT_FALSE(result.has_value());
}

/// @brief Verify that loginUser works for MECHANIC role.
TEST_F(AuthServiceTests, LoginUser_MechanicRole)
{
  DatabaseManager db;
  AuthService auth(db);

  auth.registerUser("Eve Mech", "eve@test.com", "555", "mechpass123",
                    UserRole::MECHANIC);

  auto result = auth.loginUser("eve@test.com", "mechpass123");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->role, UserRole::MECHANIC);
}

/// @}
