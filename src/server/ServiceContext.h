#pragma once

#include "../engine/DatabaseManager.h"
#include "../engine/CustomerService.h"
#include "../engine/MechanicService.h"

/// @brief Bundles all engine-layer services so endpoint handlers can call
///        business logic instead of touching the database directly.
///
///        During the transition period, `db` is still exposed so handlers
///        that haven't been refactored yet can use it.  New/refactored
///        handlers should ONLY call the appropriate service.
struct ServiceContext
{
    DatabaseManager   &db;               // transitional — prefer services
    CustomerService   &customerService;
    MechanicService   &mechanicService;
};
