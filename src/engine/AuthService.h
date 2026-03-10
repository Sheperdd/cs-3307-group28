/**
 * @file AuthService.h
 * @brief Authentication service — registration, login, password hashing.
 */
#pragma once

#include "DatabaseManager.h"
#include "Records.h"
#include <string>
#include <optional>

/// @brief Authenticated user payload returned after login/register.
struct AuthenticatedUser
{
    UserId userId{};
    UserRole role{};
    std::string fullName;
    std::string email;
};

/// @brief Handles user registration, login, and password hashing via libsodium.
class AuthService
{
public:
    /// @param db reference to the shared DatabaseManager
    explicit AuthService(DatabaseManager &db);

    /// @brief Hash a plaintext password using Argon2id (libsodium).
    /// @param plaintext the raw password string
    /// @return hashed string suitable for storage
    std::string hashPassword(const std::string &plaintext);

    /// @brief Verify a plaintext password against a stored hash.
    /// @return true if password matches
    bool verifyPassword(const std::string &plaintext, const std::string &hash);

    /// @brief Validate inputs, hash password, create user in DB.
    /// @throws std::invalid_argument on validation failure
    /// @throws std::runtime_error on DB failure
    AuthenticatedUser registerUser(const std::string &fullName,
                                   const std::string &email,
                                   const std::string &phone,
                                   const std::string &plaintextPassword,
                                   UserRole role);

    /// @brief Attempt login with email + password.
    /// @return AuthenticatedUser on success, nullopt on bad credentials
    std::optional<AuthenticatedUser> loginUser(const std::string &email,
                                               const std::string &plaintextPassword);

private:
    DatabaseManager &db_; ///< shared DB instance
};
