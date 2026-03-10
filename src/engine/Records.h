/**
 * @file Records.h
 * @brief Database record structs, ID type aliases, enums, and update payloads.
 *
 * These types map directly to DB rows. DTOs for the HTTP layer live in DTO.h.
 */
#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

// ----------- ID Types -----------
using UserId = int64_t;        ///< unique user identifier
using VehicleId = int64_t;     ///< unique vehicle identifier
using SymptomFormId = int64_t; ///< unique symptom form identifier
using MechanicId = int64_t;    ///< unique mechanic identifier
using AppointmentId = int64_t; ///< unique appointment identifier
using JobId = int64_t;         ///< unique job identifier
using JobNoteId = int64_t;     ///< unique job note identifier
using ReviewId = int64_t;      ///< unique review identifier

// ----------- Enums -----------

/// @brief User account role.
enum class UserRole {
    CUSTOMER,  ///< regular customer
    MECHANIC   ///< mechanic / shop owner
};

/// @brief Lifecycle states for an appointment.
enum class AppointmentStatus {
    REQUESTED,   ///< customer submitted request
    CONFIRMED,   ///< mechanic accepted
    SCHEDULED,   ///< date/time locked in
    IN_PROGRESS, ///< work underway
    CANCELLED,   ///< either party cancelled
    COMPLETED    ///< job finished
};

/// @brief Stages a job progresses through.
enum class JobStage {
    RECEIVED,    ///< vehicle received at shop
    DIAGNOSTICS, ///< running diagnostics
    PARTS,       ///< waiting on / ordering parts
    REPAIR,      ///< active repair work
    QA,          ///< quality assurance check
    DONE         ///< work complete
};

// ----------- Shared time structs (DB + services can reuse) -----------

/// @brief A time window with start and end ISO datetimes.
struct TimeSlot {
    std::string start; ///< ISO datetime start
    std::string end;   ///< ISO datetime end
};

/// @brief A date range filter for queries.
struct DateRange {
    std::string start; ///< ISO datetime start
    std::string end;   ///< ISO datetime end
};

// ----------- DB Records + Updates -----------

/// @brief Row from the users table.
struct UserRecord {
    UserId id{};
    std::string name;
    std::string email;
    std::string passwordHash;
    UserRole role{ UserRole::CUSTOMER };
    std::string createdAt; // ISO string
    std::string phone;
};

/// @brief Partial update payload for a user record.
struct UserUpdate {
    std::optional<std::string> fullname;
    std::optional<std::string> email;
    std::optional<std::string> phone;
    std::optional<UserRole> role;
};

/// @brief Row from the vehicles table.
struct VehicleRecord {
    VehicleId id{};
    UserId ownerUserId{};
    std::string vin;
    std::string make;
    std::string model;
    int year{ 0 };
    int mileage{ 0 };
};

/// @brief Partial update payload for a vehicle.
struct VehicleUpdate {
    std::optional<std::string> vin;
    std::optional<std::string> make;
    std::optional<std::string> model;
    std::optional<int> year;
    std::optional<int> mileage;
    UserId customerId{};
};

/// @brief Row from the symptom_forms table.
struct SymptomFormRecord {
    SymptomFormId id{};
    UserId customerId{};
    VehicleId vehicleId{};
    std::string description;
    int severity{ 0 };        // simple scale 1–5
    std::string createdAt;
};

/// @brief Partial update payload for a symptom form.
struct SymptomFormUpdate {
    std::optional<std::string> description;
    std::optional<int> severity;
};

/// @brief Row from the mechanics table.
struct MechanicRecord {
    MechanicId id{};
    UserId userId{};
    std::string displayName;
    std::string shopName;
    double hourlyRate{ 0.0 };
    std::vector<std::string> specialties; // tags like "BRAKES", "ENGINE"
};

/// @brief Partial update payload for a mechanic profile.
struct MechanicUpdate {
    std::optional<std::string> displayName;
    std::optional<std::string> shopName;
    std::optional<double> hourlyRate;
    std::optional<std::vector<std::string>> specialties;
};

/// @brief Row from the appointments table.
struct AppointmentRecord {
    UserId customerId{};
    MechanicId mechanicId{};
    std::string scheduledAt; // ISO datetime
    AppointmentStatus status{ AppointmentStatus::REQUESTED };
    std::string note;
    AppointmentId appointmentId{};
    VehicleId vehicleId{};
    std::string createdAt;
    std::string symptomForm; // what
    SymptomFormId symptomFormId;
};

/// @brief Row from the jobs table.
struct JobRecord {
    JobId id{};
    AppointmentId appointmentId{};
    MechanicId mechanicId{};
    UserId customerId{};
    VehicleId vehicleId;
    JobStage stage{ JobStage::RECEIVED };
    int percentComplete{ 0 }; // 0–100
    std::string updatedAt; // ISO datetime
    std::string startedAt;
    std::string completedAt;
};

/// @brief Single entry in a job's activity log.
struct JobNoteRecord {
    JobNoteId id{};
    JobId jobId{};
    std::string type;      // "update", "blocked", "completion"
    std::string text;
    std::string createdAt; // ISO datetime
};

/// @brief Row from the reviews table.
struct ReviewRecord {
    ReviewId id{};
    JobId jobId{};
    UserId customerId{};
    MechanicId mechanicId{};
    int rating{ 0 };
    std::string comment;
    std::string createdAt; // ISO datetime
};

/// @brief Filter criteria for mechanic search queries.
struct MechanicSearchFilter {
    std::optional<std::string> specialty;
    std::optional<double> maxDistanceKm; // can be stubbed in MVP
};
