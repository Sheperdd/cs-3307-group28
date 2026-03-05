#pragma once

#include "DTO.h"
#include "RecordsSerialization.h" // enums, Records already handled
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

// ----------- CustomerCreate (frontend → server) -----------
inline void from_json(const json &j, CustomerCreate &c)
{
    j.at("fullName").get_to(c.fullName);
    j.at("email").get_to(c.email);
    if (j.contains("phone"))
        j.at("phone").get_to(c.phone);
}

inline void to_json(json &j, const CustomerCreate &c)
{
    j = json{
        {"fullName", c.fullName},
        {"email", c.email},
        {"phone", c.phone}};
}

// ----------- CustomerDTO (server → frontend) -----------
inline void to_json(json &j, const CustomerDTO &c)
{
    j = json{
        {"userId", c.userId},
        {"fullName", c.fullName},
        {"email", c.email},
        {"phone", c.phone},
        {"createdAt", c.createdAt}};
}

inline void from_json(const json &j, CustomerDTO &c)
{
    j.at("userId").get_to(c.userId);
    j.at("fullName").get_to(c.fullName);
    j.at("email").get_to(c.email);
    if (j.contains("phone"))
        j.at("phone").get_to(c.phone);
    if (j.contains("createdAt"))
        j.at("createdAt").get_to(c.createdAt);
}

// ----------- CustomerProfileUpdate (frontend → server) -----------
inline void from_json(const json &j, CustomerProfileUpdate &u)
{
    if (j.contains("fullName"))
        u.fullName = j.at("fullName").get<std::string>();
    if (j.contains("email"))
        u.email = j.at("email").get<std::string>();
    if (j.contains("phone"))
        u.phone = j.at("phone").get<std::string>();
}

inline void to_json(json &j, const CustomerProfileUpdate &u)
{
    j = json::object();
    if (u.fullName)
        j["fullName"] = *u.fullName;
    if (u.email)
        j["email"] = *u.email;
    if (u.phone)
        j["phone"] = *u.phone;
}

// ----------- SymptomFormDTO (server → frontend) -----------
inline void to_json(json &j, const SymptomFormDTO &f)
{
    j = json{
        {"formId", f.formId},
        {"customerId", f.customerId},
        {"vehicleId", f.vehicleId},
        {"description", f.description},
        {"severity", f.severity},
        {"createdAt", f.createdAt}};
}

inline void from_json(const json &j, SymptomFormDTO &f)
{
    j.at("formId").get_to(f.formId);
    j.at("customerId").get_to(f.customerId);
    j.at("vehicleId").get_to(f.vehicleId);
    j.at("description").get_to(f.description);
    j.at("severity").get_to(f.severity);
    if (j.contains("createdAt"))
        j.at("createdAt").get_to(f.createdAt);
}

// ----------- SymptomFormCreate (frontend → server) -----------
inline void from_json(const json &j, SymptomFormCreate &f)
{
    j.at("description").get_to(f.description);
    j.at("severity").get_to(f.severity);
}

inline void to_json(json &j, const SymptomFormCreate &f)
{
    j = json{
        {"description", f.description},
        {"severity", f.severity}};
}

// ----------- AppointmentDTO (server → frontend) -----------
inline void to_json(json &j, const AppointmentDTO &a)
{
    j = json{
        {"appointmentId", a.appointmentId},
        {"customerId", a.customerId},
        {"mechanicId", a.mechanicId},
        {"formId", a.formId},
        {"customerName", a.customerName},
        {"customerEmail", a.customerEmail},
        {"customerPhone", a.customerPhone},
        {"mechanicName", a.mechanicName},
        {"scheduledAt", a.scheduledAt},
        {"status", a.status},
        {"note", a.note},
        {"createdAt", a.createdAt},
        {"vehicleId", a.vehicleId},
        {"vehicleDescription", a.vehicleDescription},
        {"symptoms", a.symptoms},
        {"severity", a.severity}};
}

inline void from_json(const json &j, AppointmentDTO &a)
{
    j.at("appointmentId").get_to(a.appointmentId);
    j.at("customerId").get_to(a.customerId);
    j.at("mechanicId").get_to(a.mechanicId);
    a.formId = j.value("formId", SymptomFormId{0});
    a.customerName = j.value("customerName", std::string{});
    a.customerEmail = j.value("customerEmail", std::string{});
    a.customerPhone = j.value("customerPhone", std::string{});
    a.mechanicName = j.value("mechanicName", std::string{});
    a.scheduledAt = j.value("scheduledAt", std::string{});
    a.status = j.value("status", AppointmentStatus::REQUESTED);
    a.note = j.value("note", std::string{});
    a.createdAt = j.value("createdAt", std::string{});
    a.vehicleId = j.value("vehicleId", VehicleId{0});
    a.vehicleDescription = j.value("vehicleDescription", std::string{});
    a.symptoms = j.value("symptoms", std::string{});
    a.severity = j.value("severity", 0);
}

inline void from_json(const json &j, AppointmentCreate &a)
{
    j.at("customerId").get_to(a.customerId);
    j.at("mechanicId").get_to(a.mechanicId);
    j.at("formId").get_to(a.formId);
    j.at("vehicleId").get_to(a.vehicleId);
    j.at("scheduledAt").get_to(a.scheduledAt);
    a.note = j.value("note", std::string{});
}

inline void to_json(json &j, const AppointmentCreate &a)
{
    j = json{
        {"customerId", a.customerId},
        {"mechanicId", a.mechanicId},
        {"formId", a.formId},
        {"vehicleId", a.vehicleId},
        {"scheduledAt", a.scheduledAt},
        {"note", a.note}};
}

// ----------- ReviewCreate (frontend → server) -----------
inline void from_json(const json &j, ReviewCreate &r)
{
    j.at("rating").get_to(r.rating);
    if (j.contains("comment"))
        j.at("comment").get_to(r.comment);
}

inline void to_json(json &j, const ReviewCreate &r)
{
    j = json{
        {"rating", r.rating},
        {"comment", r.comment}};
}

// ----------- ReviewDTO (server → frontend) -----------
inline void to_json(json &j, const ReviewDTO &r)
{
    j = json{
        {"reviewId", r.reviewId},
        {"mechanicId", r.mechanicId},
        {"customerId", r.customerId},
        {"rating", r.rating},
        {"comment", r.comment},
        {"createdAt", r.createdAt},
        {"customerName", r.customerName}};
}

inline void from_json(const json &j, ReviewDTO &r)
{
    j.at("reviewId").get_to(r.reviewId);
    j.at("mechanicId").get_to(r.mechanicId);
    j.at("customerId").get_to(r.customerId);
    j.at("rating").get_to(r.rating);
    r.comment = j.value("comment", std::string{});
    r.createdAt = j.value("createdAt", std::string{});
    r.customerName = j.value("customerName", std::string{});
}

// ----------- MechanicDTO (server → frontend) -----------
inline void to_json(json &j, const MechanicDTO &m)
{
    j = json{
        {"mechanicId", m.mechanicId},
        {"userId", m.userId},
        {"displayName", m.displayName},
        {"shopName", m.shopName},
        {"hourlyRate", m.hourlyRate},
        {"specialties", m.specialties},
        {"averageRating", m.averageRating},
        {"reviewCount", m.reviewCount}};
}

inline void from_json(const json &j, MechanicDTO &m)
{
    j.at("mechanicId").get_to(m.mechanicId);
    m.userId = j.value("userId", UserId{0});
    j.at("displayName").get_to(m.displayName);
    m.shopName = j.value("shopName", std::string{});
    m.hourlyRate = j.value("hourlyRate", 0.0);
    if (j.contains("specialties"))
        j.at("specialties").get_to(m.specialties);
    m.averageRating = j.value("averageRating", 0.0);
    m.reviewCount = j.value("reviewCount", 0);
}

// ----------- MechanicMatch (server → frontend) -----------
inline void to_json(json &j, const MechanicMatch &m)
{
    j = json{
        {"mechanicId", m.mechanicId},
        {"matchScore", m.matchScore},
        {"reasons", m.reasons}};
}

inline void from_json(const json &j, MechanicMatch &m)
{
    j.at("mechanicId").get_to(m.mechanicId);
    m.matchScore = j.value("matchScore", 0.0);
    if (j.contains("reasons"))
        j.at("reasons").get_to(m.reasons);
}

// ----------- MechanicProfileUpdate (frontend → server) -----------
inline void from_json(const json &j, MechanicUpdateDTO &u)
{
    u.mechanicId = j.value("mechanicId", MechanicId{0});
    if (j.contains("displayName"))
        u.displayName = j.at("displayName").get<std::string>();
    if (j.contains("shopName"))
        u.shopName = j.at("shopName").get<std::string>();
    if (j.contains("hourlyRate"))
        u.hourlyRate = j.at("hourlyRate").get<double>();
    if (j.contains("specialties"))
        u.specialties = j.at("specialties").get<std::vector<std::string>>();
}

inline void to_json(json &j, const MechanicUpdateDTO &u)
{
    j = json{
        {"mechanicId", u.mechanicId},
        {"displayName", u.displayName.value_or(std::string{})},
        {"shopName", u.shopName.value_or(std::string{})},
        {"hourlyRate", u.hourlyRate.value_or(0.0)},
        {"specialties", u.specialties.value_or(std::vector<std::string>{})}};
}

// ----------- PriceEstimate (server → frontend) -----------
inline void to_json(json &j, const PriceEstimate &p)
{
    j = json{
        {"laborCost", p.laborCost},
        {"partsCost", p.partsCost},
        {"tax", p.tax},
        {"total", p.total},
        {"note", p.note}};
}

inline void from_json(const json &j, PriceEstimate &p)
{
    j.at("laborCost").get_to(p.laborCost);
    j.at("partsCost").get_to(p.partsCost);
    j.at("tax").get_to(p.tax);
    j.at("total").get_to(p.total);
    p.note = j.value("note", std::string{});
}

// ----------- JobNoteDTO (server → frontend) -----------
inline void to_json(json &j, const JobNoteDTO &n)
{
    j = json{
        {"noteId", n.noteId},
        {"type", n.type},
        {"text", n.text},
        {"createdAt", n.createdAt}};
}

inline void from_json(const json &j, JobNoteDTO &n)
{
    n.noteId = j.value("noteId", JobNoteId{0});
    n.type = j.value("type", std::string{"update"});
    n.text = j.value("text", std::string{});
    n.createdAt = j.value("createdAt", std::string{});
}

// ----------- JobDTO (server → frontend) -----------
inline void to_json(json &j, const JobDTO &jd)
{
    j = json{
        {"jobId", jd.jobId},
        {"appointmentId", jd.appointmentId},
        {"customerId", jd.customerId},
        {"mechanicId", jd.mechanicId},
        {"currentStage", jd.currentStage},
        {"percentComplete", jd.percentComplete},
        {"notes", jd.notes},
        {"updatedAt", jd.updatedAt},
        {"startedAt", jd.startedAt},
        {"completedAt", jd.completedAt},
        {"customerName", jd.customerName},
        {"customerEmail", jd.customerEmail},
        {"vehicleDescription", jd.vehicleDescription},
        {"isBlocked", jd.isBlocked}};
}

inline void from_json(const json &j, JobDTO &jd)
{
    j.at("jobId").get_to(jd.jobId);
    jd.appointmentId = j.value("appointmentId", AppointmentId{0});
    jd.customerId = j.value("customerId", UserId{0});
    jd.mechanicId = j.value("mechanicId", MechanicId{0});
    jd.currentStage = j.value("currentStage", JobStage::RECEIVED);
    jd.percentComplete = j.value("percentComplete", 0);
    if (j.contains("notes")) {
        jd.notes = j.at("notes").get<std::vector<JobNoteDTO>>();
    }
    jd.updatedAt = j.value("updatedAt", std::string{});
    jd.startedAt = j.value("startedAt", std::string{});
    jd.completedAt = j.value("completedAt", std::string{});
    jd.customerName = j.value("customerName", std::string{});
    jd.customerEmail = j.value("customerEmail", std::string{});
    jd.vehicleDescription = j.value("vehicleDescription", std::string{});
    jd.isBlocked = j.value("isBlocked", false);
}

// ----------- Session (server → frontend) -----------
inline void to_json(json &j, const Session &s)
{
    j = json{
        {"sessionId", s.sessionId},
        {"userId", s.userId},
        {"role", s.role},
        {"createdAt", s.createdAt},
        {"expiresAt", s.expiresAt}};
}

// ----------- AuthResult (server → frontend) -----------
inline void to_json(json &j, const AuthResult &a)
{
    j = json{
        {"success", a.success},
        {"message", a.message}};
    if (a.session.has_value())
        j["session"] = *a.session;
}

// ----------- MechanicCreate (frontend → server) -----------
inline void from_json(const json &j, MechanicCreate &m)
{
    j.at("displayName").get_to(m.displayName);
    m.shopName = j.value("shopName", std::string{});
    m.hourlyRate = j.value("hourlyRate", 0.0);
    if (j.contains("specialties"))
        j.at("specialties").get_to(m.specialties);
}

inline void to_json(json &j, const MechanicCreate &m)
{
    j = json{
        {"displayName", m.displayName},
        {"shopName", m.shopName},
        {"hourlyRate", m.hourlyRate},
        {"specialties", m.specialties}};
}
