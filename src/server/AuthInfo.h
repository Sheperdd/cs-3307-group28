/**
 * @file AuthInfo.h
 * @brief Lightweight struct carrying the authenticated user's identity.
 */
#pragma once

#include "../engine/Records.h"

/// @brief Identity extracted from a verified JWT token.
struct AuthInfo
{
    UserId userId{};
    UserRole role{UserRole::CUSTOMER};
};
