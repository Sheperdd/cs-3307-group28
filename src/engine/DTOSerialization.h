#pragma once

#include "DTO.h"
#include "RecordsSerialization.h"   // enums, Records already handled
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// =====================================================================
//  DTO  ⇄  JSON   (server ↔ frontend)
//
//  Add to_json / from_json for each DTO as you refactor endpoints
//  to go through the engine layer.
// =====================================================================

// ----------- VehicleCreate (frontend → server) -----------
inline void from_json(const json &j, VehicleCreate &v)
{
    j.at("vin").get_to(v.vin);
    j.at("make").get_to(v.make);
    j.at("model").get_to(v.model);
    j.at("year").get_to(v.year);
    if (j.contains("mileage"))
        j.at("mileage").get_to(v.mileage);
}

inline void to_json(json &j, const VehicleCreate &v)
{
    j = json{
        {"vin", v.vin},
        {"make", v.make},
        {"model", v.model},
        {"year", v.year},
        {"mileage", v.mileage}};
}

// ----------- VehicleDTO (server → frontend) -----------
inline void to_json(json &j, const VehicleDTO &v)
{
    j = json{
        {"vehicleId", v.vehicleId},
        {"ownerId", v.ownerId},
        {"vin", v.vin},
        {"make", v.make},
        {"model", v.model},
        {"year", v.year},
        {"mileage", v.mileage}};
}

inline void from_json(const json &j, VehicleDTO &v)
{
    j.at("vehicleId").get_to(v.vehicleId);
    j.at("ownerId").get_to(v.ownerId);
    j.at("vin").get_to(v.vin);
    j.at("make").get_to(v.make);
    j.at("model").get_to(v.model);
    j.at("year").get_to(v.year);
    if (j.contains("mileage"))
        j.at("mileage").get_to(v.mileage);
}
