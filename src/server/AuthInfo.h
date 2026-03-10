#pragma once

#include "../engine/Records.h"

struct AuthInfo
{
    UserId userId{};
    UserRole role{UserRole::CUSTOMER};
};
