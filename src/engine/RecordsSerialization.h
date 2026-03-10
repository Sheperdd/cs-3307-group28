/**
 * @file RecordsSerialization.h
 * @brief nlohmann::json to_json / from_json overloads for Record structs and enums.
 *
 * Enum values are serialized as human-readable strings (e.g. "CUSTOMER", "MECHANIC").
 */
#pragma once

#include "Records.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

// =====================================================================
//  Enum  ⇄  JSON   (serialized as human-readable strings)
// =====================================================================

// ----------- UserRole -----------
inline void to_json(json &j, UserRole role)
{
  switch (role)
  {
  case UserRole::CUSTOMER:
    j = "CUSTOMER";
    break;
  case UserRole::MECHANIC:
    j = "MECHANIC";
    break;
  }
}

inline void from_json(const json &j, UserRole &role)
{
  const auto s = j.get<std::string>();
  if (s == "CUSTOMER")
    role = UserRole::CUSTOMER;
  else if (s == "MECHANIC")
    role = UserRole::MECHANIC;
  else
    throw json::other_error::create(501, "Unknown UserRole: " + s, &j);
}

// ----------- AppointmentStatus -----------
inline void to_json(json &j, AppointmentStatus s)
{
  switch (s)
  {
  case AppointmentStatus::REQUESTED:
    j = "REQUESTED";
    break;
  case AppointmentStatus::SCHEDULED:
    j = "SCHEDULED";
    break;
  case AppointmentStatus::CANCELLED:
    j = "CANCELLED";
    break;
  case AppointmentStatus::COMPLETED:
    j = "COMPLETED";
    break;
  case AppointmentStatus::CONFIRMED:
    j = "CONFIRMED";
    break;
  case AppointmentStatus::IN_PROGRESS:
    j = "IN_PROGRESS";
    break;
  }
}

inline void from_json(const json &j, AppointmentStatus &s)
{
  const auto v = j.get<std::string>();
  if (v == "REQUESTED")
    s = AppointmentStatus::REQUESTED;
  else if (v == "CONFIRMED")
    s = AppointmentStatus::CONFIRMED;
  else if (v == "SCHEDULED")
    s = AppointmentStatus::SCHEDULED;
  else if (v == "IN_PROGRESS")
    s = AppointmentStatus::IN_PROGRESS;
  else if (v == "CANCELLED")
    s = AppointmentStatus::CANCELLED;
  else if (v == "COMPLETED")
    s = AppointmentStatus::COMPLETED;
  else
    throw json::other_error::create(501, "Unknown AppointmentStatus: " + v, &j);
}

// ----------- JobStage -----------
inline void to_json(json &j, JobStage s)
{
  switch (s)
  {
  case JobStage::RECEIVED:
    j = "RECEIVED";
    break;
  case JobStage::DIAGNOSTICS:
    j = "DIAGNOSTICS";
    break;
  case JobStage::PARTS:
    j = "PARTS";
    break;
  case JobStage::REPAIR:
    j = "REPAIR";
    break;
  case JobStage::QA:
    j = "QA";
    break;
  case JobStage::DONE:
    j = "DONE";
    break;
  }
}

inline void from_json(const json &j, JobStage &s)
{
  const auto v = j.get<std::string>();
  if (v == "RECEIVED")
    s = JobStage::RECEIVED;
  else if (v == "DIAGNOSTICS")
    s = JobStage::DIAGNOSTICS;
  else if (v == "PARTS")
    s = JobStage::PARTS;
  else if (v == "REPAIR")
    s = JobStage::REPAIR;
  else if (v == "QA")
    s = JobStage::QA;
  else if (v == "DONE")
    s = JobStage::DONE;
  else
    throw json::other_error::create(501, "Unknown JobStage: " + v, &j);
}

// =====================================================================
//  Shared time structs  ⇄  JSON
// =====================================================================

// ----------- TimeSlot -----------
inline void to_json(json &j, const TimeSlot &t)
{
  j = json{{"start", t.start}, {"end", t.end}};
}

inline void from_json(const json &j, TimeSlot &t)
{
  j.at("start").get_to(t.start);
  j.at("end").get_to(t.end);
}

// ----------- DateRange -----------
inline void to_json(json &j, const DateRange &d)
{
  j = json{{"start", d.start}, {"end", d.end}};
}

inline void from_json(const json &j, DateRange &d)
{
  j.at("start").get_to(d.start);
  j.at("end").get_to(d.end);
}

// =====================================================================
//  Record structs  ⇄  JSON
// =====================================================================

// ----------- UserRecord -----------
inline void to_json(json &j, const UserRecord &u)
{
  j = json{
      {"id", u.id},
      {"name", u.name},
      {"phone", u.phone},
      {"email", u.email},
      {"passwordHash", u.passwordHash},
      {"role", u.role}, // uses UserRole to_json above
      {"createdAt", u.createdAt}};
}

inline void from_json(const json &j, UserRecord &u)
{
  u.id = j.value("id", UserId{0});
  u.name = j.value("name", std::string{});
  u.phone = j.value("phone", std::string{});
  j.at("email").get_to(u.email);
  j.at("passwordHash").get_to(u.passwordHash);
  u.role = j.value("role", UserRole::CUSTOMER);
  u.createdAt = j.value("createdAt", std::string{});
}

// ----------- VehicleRecord -----------
inline void to_json(json &j, const VehicleRecord &v)
{
  j = json{
      {"id", v.id},
      {"ownerUserId", v.ownerUserId},
      {"vin", v.vin},
      {"make", v.make},
      {"model", v.model},
      {"year", v.year},
      {"mileage", v.mileage}};
}

inline void from_json(const json &j, VehicleRecord &v)
{
  v.id = j.value("id", VehicleId{0});
  v.ownerUserId = j.value("ownerUserId", UserId{0});
  j.at("vin").get_to(v.vin);
  j.at("make").get_to(v.make);
  j.at("model").get_to(v.model);
  j.at("year").get_to(v.year);
  j.at("mileage").get_to(v.mileage);
}

// ----------- SymptomFormRecord -----------
inline void to_json(json &j, const SymptomFormRecord &f)
{
  j = json{
      {"id", f.id},
      {"customerId", f.customerId},
      {"vehicleId", f.vehicleId},
      {"description", f.description},
      {"severity", f.severity},
      {"createdAt", f.createdAt}};
}

inline void from_json(const json &j, SymptomFormRecord &f)
{
  f.id = j.value("id", SymptomFormId{0});
  f.customerId = j.value("customerId", UserId{0});
  f.vehicleId = j.value("vehicleId", VehicleId{0});
  j.at("description").get_to(f.description);
  j.at("severity").get_to(f.severity);
  f.createdAt = j.value("createdAt", std::string{});
}

// ----------- MechanicRecord -----------
inline void to_json(json &j, const MechanicRecord &m)
{
  j = json{
      {"id", m.id},
      {"userId", m.userId},
      {"displayName", m.displayName},
      {"shopName", m.shopName},
      {"hourlyRate", m.hourlyRate},
      {"specialties", m.specialties}};
}

inline void from_json(const json &j, MechanicRecord &m)
{
  m.id = j.value("id", MechanicId{0});
  m.userId = j.value("userId", UserId{0});
  j.at("displayName").get_to(m.displayName);
  j.at("shopName").get_to(m.shopName);
  j.at("hourlyRate").get_to(m.hourlyRate);
  j.at("specialties").get_to(m.specialties);
}

// ----------- AppointmentRecord -----------
inline void to_json(json &j, const AppointmentRecord &a)
{
  j = json{
      {"id", a.appointmentId},
      {"customerId", a.customerId},
      {"mechanicId", a.mechanicId},
      {"formId", a.symptomFormId},
      {"scheduledAt", a.scheduledAt},
      {"status", a.status}, // uses AppointmentStatus to_json
      {"note", a.note}};
}

inline void from_json(const json &j, AppointmentRecord &a)
{
  a.appointmentId = j.value("id", AppointmentId{0});
  a.customerId = j.value("customerId", UserId{0});
  a.mechanicId = j.value("mechanicId", MechanicId{0});
  a.symptomFormId = j.value("formId", SymptomFormId{0});
  a.scheduledAt = j.value("scheduledAt", std::string{});
  a.status = j.value("status", AppointmentStatus::REQUESTED);
  a.note = j.value("note", std::string{});
}

// ----------- JobRecord -----------
inline void to_json(json &j, const JobRecord &jr)
{
  j = json{
      {"id", jr.id},
      {"appointmentId", jr.appointmentId},
      {"mechanicId", jr.mechanicId},
      {"customerId", jr.customerId},
      {"stage", jr.stage}, // uses JobStage to_json
      {"percentComplete", jr.percentComplete},
      {"updatedAt", jr.updatedAt},
      {"startedAt", jr.startedAt},
      {"completedAt", jr.completedAt}};
}

inline void from_json(const json &j, JobRecord &jr)
{
  jr.id = j.value("id", JobId{0});
  jr.appointmentId = j.value("appointmentId", AppointmentId{0});
  jr.mechanicId = j.value("mechanicId", MechanicId{0});
  jr.customerId = j.value("customerId", UserId{0});
  jr.stage = j.value("stage", JobStage::RECEIVED);
  jr.percentComplete = j.value("percentComplete", 0);
  jr.updatedAt = j.value("updatedAt", std::string{});
  jr.startedAt = j.value("startedAt", std::string{});
  jr.completedAt = j.value("completedAt", std::string{});
}

// ----------- JobNoteRecord -----------
inline void to_json(json &j, const JobNoteRecord &n)
{
  j = json{
      {"id", n.id},
      {"jobId", n.jobId},
      {"type", n.type},
      {"text", n.text},
      {"createdAt", n.createdAt}};
}

inline void from_json(const json &j, JobNoteRecord &n)
{
  n.id = j.value("id", JobNoteId{0});
  n.jobId = j.value("jobId", JobId{0});
  n.type = j.value("type", std::string{"update"});
  n.text = j.value("text", std::string{});
  n.createdAt = j.value("createdAt", std::string{});
}

// ----------- ReviewRecord -----------
inline void to_json(json &j, const ReviewRecord &r)
{
  j = json{
      {"id", r.id},
      {"jobId", r.jobId},
      {"customerId", r.customerId},
      {"mechanicId", r.mechanicId},
      {"rating", r.rating},
      {"comment", r.comment},
      {"createdAt", r.createdAt}};
}

inline void from_json(const json &j, ReviewRecord &r)
{
  r.id = j.value("id", ReviewId{0});
  r.jobId = j.value("jobId", JobId{0});
  r.customerId = j.value("customerId", UserId{0});
  r.mechanicId = j.value("mechanicId", MechanicId{0});
  j.at("rating").get_to(r.rating);
  r.comment = j.value("comment", std::string{});
  r.createdAt = j.value("createdAt", std::string{});
}

// =====================================================================
//  Update structs  ⇄  JSON   (partial – only set fields are included)
// =====================================================================

// ----------- UserUpdate -----------
inline void to_json(json &j, const UserUpdate &u)
{
  j = json::object();
  if (u.email)
    j["email"] = *u.email;
  if (u.role)
    j["role"] = *u.role;
}

inline void from_json(const json &j, UserUpdate &u)
{
  if (j.contains("email"))
    u.email = j.at("email").get<std::string>();
  if (j.contains("role"))
    u.role = j.at("role").get<UserRole>();
}

// ----------- VehicleUpdate -----------
inline void to_json(json &j, const VehicleUpdate &v)
{
  j = json::object();
  if (v.vin)
    j["vin"] = *v.vin;
  if (v.make)
    j["make"] = *v.make;
  if (v.model)
    j["model"] = *v.model;
  if (v.year)
    j["year"] = *v.year;
  if (v.mileage)
    j["mileage"] = *v.mileage;
}

inline void from_json(const json &j, VehicleUpdate &v)
{
  if (j.contains("vin"))
    v.vin = j.at("vin").get<std::string>();
  if (j.contains("make"))
    v.make = j.at("make").get<std::string>();
  if (j.contains("model"))
    v.model = j.at("model").get<std::string>();
  if (j.contains("year"))
    v.year = j.at("year").get<int>();
  if (j.contains("mileage"))
    v.mileage = j.at("mileage").get<int>();
}

// ----------- SymptomFormUpdate -----------
inline void to_json(json &j, const SymptomFormUpdate &f)
{
  j = json::object();
  if (f.description)
    j["description"] = *f.description;
  if (f.severity)
    j["severity"] = *f.severity;
}

inline void from_json(const json &j, SymptomFormUpdate &f)
{
  if (j.contains("description"))
    f.description = j.at("description").get<std::string>();
  if (j.contains("severity"))
    f.severity = j.at("severity").get<int>();
}

// ----------- MechanicUpdate -----------
inline void to_json(json &j, const MechanicUpdate &m)
{
  j = json::object();
  if (m.displayName)
    j["displayName"] = *m.displayName;
  if (m.shopName)
    j["shopName"] = *m.shopName;
  if (m.hourlyRate)
    j["hourlyRate"] = *m.hourlyRate;
  if (m.specialties)
    j["specialties"] = *m.specialties;
}

inline void from_json(const json &j, MechanicUpdate &m)
{
  if (j.contains("displayName"))
    m.displayName = j.at("displayName").get<std::string>();
  if (j.contains("shopName"))
    m.shopName = j.at("shopName").get<std::string>();
  if (j.contains("hourlyRate"))
    m.hourlyRate = j.at("hourlyRate").get<double>();
  if (j.contains("specialties"))
    m.specialties = j.at("specialties").get<std::vector<std::string>>();
}

// =====================================================================
//  Filter structs  ⇄  JSON
// =====================================================================

// ----------- MechanicSearchFilter -----------
inline void to_json(json &j, const MechanicSearchFilter &f)
{
  j = json::object();
  if (f.specialty)
    j["specialty"] = *f.specialty;
  if (f.maxDistanceKm)
    j["maxDistanceKm"] = *f.maxDistanceKm;
}

inline void from_json(const json &j, MechanicSearchFilter &f)
{
  if (j.contains("specialty"))
    f.specialty = j.at("specialty").get<std::string>();
  if (j.contains("maxDistanceKm"))
    f.maxDistanceKm = j.at("maxDistanceKm").get<double>();
}
