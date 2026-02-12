#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

#include "Records.h" // IDs, enums, TimeSlot, DateRange, AppointmentStatus, JobStage, UserRole, etc.

// ------------------ Extra Shared Types ------------------

using SubscriptionId = int64_t; // can change later if you want UUID strings
using SessionId = int64_t;      // keep simple for MVP (can switch to UUID string later)


// If you want discovery filters later, keep it minimal for now
struct DiscoveryFilter {
    std::optional<std::string> specialty;
    std::optional<double> maxDistanceKm; // stub OK MVP
};

// ------------------ Auth DTOs ------------------

struct Session {
    SessionId sessionId{};
    UserId userId{};
    UserRole role{ UserRole::CUSTOMER };
    std::string createdAt;   // ISO datetime
    std::string expiresAt;   // ISO datetime (optional usage)
};

struct AuthResult {
    bool success{ false };
    std::string message; // "ok", "invalid credentials", "role not allowed"
    std::optional<Session> session;
};

// ------------------ Customer Profile DTOs ------------------

struct CustomerProfileCreate {
    std::string fullName;
    std::string email;
    std::string phone;
};

struct CustomerProfile {
    UserId customerId{};
    std::string fullName;
    std::string email;
    std::string phone;
    std::string createdAt; // ISO datetime
};

struct CustomerProfileUpdate {
    std::optional<std::string> fullName;
    std::optional<std::string> email;
    std::optional<std::string> phone;
};

// ------------------ Vehicle DTOs (Service-facing) ------------------

struct VehicleCreate {
    std::string vin;
    std::string make;
    std::string model;
    int year{ 0 };
    int mileage{ 0 };
};

struct VehicleSummary {
    VehicleId vehicleId{};
    std::string make;
    std::string model;
    int year{ 0 };
};

// ------------------ Symptom Form DTOs ------------------

struct SymptomFormCreate {
    std::string description;
    int severity{ 0 }; // 1–5
};

struct SymptomFormSummary {
    SymptomFormId formId{};
    VehicleId vehicleId{};
    std::string shortDescription;
    int severity{ 0 };
    std::string createdAt; // ISO datetime
};

struct SymptomFormDetails {
    SymptomFormId formId{};
    UserId customerId{};
    VehicleId vehicleId{};
    std::string description;
    int severity{ 0 };
    std::string createdAt; // ISO datetime
};

// ------------------ Discovery / Mechanic Views ------------------

struct MechanicProfileView {
    MechanicId mechanicId{};
    std::string displayName;
    std::string shopName;
    double hourlyRate{ 0.0 };
    std::vector<std::string> specialties;

    double averageRating{ 0.0 };
    int reviewCount{ 0 };
    std::optional<double> distanceKm; // stub OK MVP
};

struct MechanicMatch {
    MechanicId mechanicId{};
    double matchScore{ 0.0 };
    std::vector<std::string> reasons;
};

// ------------------ Mechanic Profile (MechanicService-facing) ------------------

struct MechanicProfileCreate {
    std::string displayName;
    std::string shopName;
    double hourlyRate{ 0.0 };
    std::vector<std::string> specialties;
};

struct MechanicProfile {
    MechanicId mechanicId{};
    UserId userId{};
    std::string displayName;
    std::string shopName;
    double hourlyRate{ 0.0 };
    std::vector<std::string> specialties;
};

struct MechanicProfileUpdate {
    std::optional<std::string> displayName;
    std::optional<std::string> shopName;
    std::optional<double> hourlyRate;
    std::optional<std::vector<std::string>> specialties;
};

// ------------------ Pricing / Estimate ------------------

struct PriceEstimate {
    int laborCost{};
    int partsCost{};
    int tax{};
    int total{};
    std::string note;
};

// ------------------ Appointments ------------------

struct AppointmentRequestView {
    AppointmentId appointmentId{};
    UserId customerId{};
    SymptomFormId formId{};
    std::string requestedAt; // ISO datetime
    std::string summary;
};

struct AppointmentSummary {
    AppointmentId appointmentId{};
    MechanicId mechanicId{};
    std::string scheduledAt; // ISO datetime
    AppointmentStatus status{ AppointmentStatus::REQUESTED };
};

struct AppointmentDetails {
    AppointmentId appointmentId{};
    UserId customerId{};
    MechanicId mechanicId{};
    SymptomFormId formId{};
    std::string scheduledAt; // ISO datetime
    int durationMinutes{ 0 };
    AppointmentStatus status{ AppointmentStatus::REQUESTED };
    std::string note;
};

// ------------------ Jobs ------------------

struct JobStatusView {
    JobId jobId{};
    JobStage stage{ JobStage::RECEIVED };
    int percentComplete{ 0 };
    std::string lastNote;
    std::string updatedAt; // ISO datetime
};

struct JobCardView {
    JobId jobId{};
    UserId customerId{};
    JobStage stage{ JobStage::RECEIVED };
    int percentComplete{ 0 };
    std::string updatedAt; // ISO datetime
};

struct JobDetailsView {
    JobId jobId{};
    AppointmentId appointmentId{};
    UserId customerId{};
    MechanicId mechanicId{};
    JobStage stage{ JobStage::RECEIVED };
    int percentComplete{ 0 };
    std::string lastNote;
    std::string updatedAt; // ISO datetime
};

// ------------------ Reviews ------------------

struct ReviewCreate {
    int rating{ 0 }; // 1–5
    std::string comment;
};

struct ReviewSummary {
    ReviewId reviewId{};
    MechanicId mechanicId{};
    UserId customerId{};
    int rating{ 0 };
    std::string comment;
    std::string createdAt; // ISO datetime
};
