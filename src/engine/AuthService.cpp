#include "AuthService.h"
#include <sodium.h>
#include <stdexcept>

AuthService::AuthService(DatabaseManager &db) : db_(db)
{
    if (sodium_init() < 0)
        throw std::runtime_error("AuthService: failed to initialize libsodium");
}

std::string AuthService::hashPassword(const std::string &plaintext)
{
    char hash[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(hash,
                          plaintext.c_str(), plaintext.size(),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
    {
        throw std::runtime_error("hashPassword: out of memory");
    }
    return std::string(hash);
}

bool AuthService::verifyPassword(const std::string &plaintext, const std::string &hash)
{
    return crypto_pwhash_str_verify(hash.c_str(),
                                    plaintext.c_str(), plaintext.size()) == 0;
}

AuthenticatedUser AuthService::registerUser(const std::string &fullName,
                                             const std::string &email,
                                             const std::string &phone,
                                             const std::string &plaintextPassword,
                                             UserRole role)
{
    if (fullName.empty())
        throw std::invalid_argument("registerUser: name is required");
    if (email.empty())
        throw std::invalid_argument("registerUser: email is required");
    if (plaintextPassword.size() < 8)
        throw std::invalid_argument("registerUser: password must be at least 8 characters");
    if (db_.emailExists(email))
        throw std::invalid_argument("registerUser: email already registered");

    std::string hash = hashPassword(plaintextPassword);

    UserRecord rec{};
    rec.name = fullName;
    rec.email = email;
    rec.phone = phone;
    rec.passwordHash = hash;
    rec.role = role;

    UserId userId = db_.createUser(rec);
    if (userId <= 0)
        throw std::runtime_error("registerUser: failed to create user");

    return AuthenticatedUser{userId, role, fullName, email};
}

std::optional<AuthenticatedUser> AuthService::loginUser(const std::string &email,
                                                         const std::string &plaintextPassword)
{
    auto user = db_.getUserRecordByEmail(email);
    if (!user.has_value())
        return std::nullopt;

    if (!verifyPassword(plaintextPassword, user->passwordHash))
        return std::nullopt;

    return AuthenticatedUser{user->id, user->role, user->name, user->email};
}
