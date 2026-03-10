#pragma once

#include "DatabaseManager.h"
#include "Records.h"
#include <string>
#include <optional>

struct AuthenticatedUser
{
    UserId userId{};
    UserRole role{};
    std::string fullName;
    std::string email;
};

class AuthService
{
public:
    explicit AuthService(DatabaseManager &db);

    std::string hashPassword(const std::string &plaintext);
    bool verifyPassword(const std::string &plaintext, const std::string &hash);

    // Validates, hashes password, creates user in DB. Throws on failure.
    AuthenticatedUser registerUser(const std::string &fullName,
                                   const std::string &email,
                                   const std::string &phone,
                                   const std::string &plaintextPassword,
                                   UserRole role);

    // Returns nullopt if credentials are invalid.
    std::optional<AuthenticatedUser> loginUser(const std::string &email,
                                               const std::string &plaintextPassword);

private:
    DatabaseManager &db_;
};
