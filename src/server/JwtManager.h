/**
 * @file JwtManager.h
 * @brief Stateless JWT helpers — token generation, verification, and
 *        secret management via environment variable.
 */
#pragma once

#include "AuthInfo.h"
#include <jwt-cpp/jwt.h>
#include <string>
#include <optional>
#include <cstdlib>

namespace JwtManager
{

inline const std::string &getSecret()
{
    static const std::string secret = []()
    {
        const char *env = std::getenv("TORQUEDESK_JWT_SECRET");
        return env ? std::string(env)
                   : std::string("torquedesk-dev-secret-change-in-production");
    }();
    return secret;
}

inline std::string generateToken(UserId userId, UserRole role)
{
    auto token = jwt::create()
                     .set_issuer("torquedesk")
                     .set_subject(std::to_string(userId))
                     .set_payload_claim("role",
                                        jwt::claim(std::string(
                                            role == UserRole::MECHANIC ? "mechanic" : "customer")))
                     .set_issued_at(std::chrono::system_clock::now())
                     .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24))
                     .sign(jwt::algorithm::hs256{getSecret()});
    return token;
}

inline std::optional<AuthInfo> verifyToken(const std::string &token)
{
    try
    {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{getSecret()})
                            .with_issuer("torquedesk");
        verifier.verify(decoded);

        AuthInfo info;
        info.userId = std::stoll(decoded.get_subject());
        auto roleStr = decoded.get_payload_claim("role").as_string();
        info.role = (roleStr == "mechanic") ? UserRole::MECHANIC : UserRole::CUSTOMER;
        return info;
    }
    catch (...)
    {
        return std::nullopt;
    }
}

} // namespace JwtManager
