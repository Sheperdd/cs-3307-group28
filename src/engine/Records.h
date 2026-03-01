#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

// ----------- ID Types -----------
using UserId = int64_t;
using VehicleId = int64_t;
using SymptomFormId = int64_t;
using MechanicId = int64_t;
using AppointmentId = int64_t;
using JobId = int64_t;
using ReviewId = int64_t;

// ----------- Enums -----------
enum class UserRole {
    CUSTOMER,
    MECHANIC
};

enum class AppointmentStatus {
    REQUESTED,
    CONFIRMED,
    SCHEDULED,
    IN_PROGRESS,
    CANCELLED,
    COMPLETED
};

enum class JobStage {
    RECEIVED,
    DIAGNOSTICS,
    PARTS,
    REPAIR,
    QA,
    DONE
};

// ----------- Shared time structs (DB + services can reuse) -----------
struct TimeSlot {
    std::string start; // ISO datetime
    std::string end;   // ISO datetime
};

struct DateRange {
    std::string start; // ISO datetime
    std::string end;   // ISO datetime
};

// ----------- DB Records + Updates -----------

// User
struct UserRecord {
    UserId id{};
    std::string name;
    std::string email;
    std::string passwordHash;
    UserRole role{ UserRole::CUSTOMER };
    std::string createdAt; // ISO string
    std::string phone;
};

struct UserUpdate {
    std::optional<std::string> email;
    std::optional<UserRole> role;
};

// Vehicle
struct VehicleRecord {
    VehicleId id{};
    UserId ownerUserId{};
    std::string vin;
    std::string make;
    std::string model;
    int year{ 0 };
    int mileage{ 0 };
};

struct VehicleUpdate {
    std::optional<std::string> vin;
    std::optional<std::string> make;
    std::optional<std::string> model;
    std::optional<int> year;
    std::optional<int> mileage;
};

// Symptom Form
struct SymptomFormRecord {
    SymptomFormId id{};
    UserId customerId{};
    VehicleId vehicleId{};
    std::string description;
    int severity{ 0 };        // simple scale 1–5
    std::string createdAt;
};

struct SymptomFormUpdate {
    std::optional<std::string> description;
    std::optional<int> severity;
};

// Mechanic
struct MechanicRecord {
    MechanicId id{};
    UserId userId{};
    std::string displayName;
    std::string shopName;
    double hourlyRate{ 0.0 };
    std::vector<std::string> specialties; // tags like "BRAKES", "ENGINE"
};

struct MechanicUpdate {
    std::optional<std::string> displayName;
    std::optional<std::string> shopName;
    std::optional<double> hourlyRate;
    std::optional<std::vector<std::string>> specialties;
};

// Appointment
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

// Job
struct JobRecord {
    JobId id{};
    AppointmentId appointmentId{};
    MechanicId mechanicId{};
    UserId customerId{};
    VehicleId vehicleId;
    JobStage stage{ JobStage::RECEIVED };
    int percentComplete{ 0 }; // 0–100
    std::string lastNote;
    std::string updatedAt; // ISO datetime
    JobStage currentStage;
    std::string startedAt;
    std::string completedAt;
    std::string completionNote;
};

// Review
struct ReviewRecord {
    ReviewId id{};
    JobId jobId{};
    UserId customerId{};
    MechanicId mechanicId{};
    int rating{ 0 };
    std::string comment;
    std::string createdAt; // ISO datetime
};

// ----------- Filters (DB query inputs) -----------
struct MechanicSearchFilter {
    std::optional<std::string> specialty;
    std::optional<double> maxDistanceKm; // can be stubbed in MVP
};
