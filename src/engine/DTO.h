/**
 * @file DTO.h
 * @brief Data Transfer Objects for the HTTP API layer (server ↔ frontend).
 *
 * One DTO per table, plus auth helpers. DB-layer Record/Update structs
 * live in Records.h — don't duplicate them here.
 *
 * Tables (7): Customers, Vehicles, SymptomForms, Mechanics,
 *             Appointments, Jobs, Reviews
 */
#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

#include "Records.h"

// ========================= Shared ID aliases =========================
using SubscriptionId = int64_t; ///< real-time subscription handle
using SessionId = int64_t;      ///< login session handle

// ========================= Auth (server → frontend) =========================

/// @brief Active session returned on login.
struct Session
{
    SessionId sessionId{};
    UserId userId{};
    UserRole role{UserRole::CUSTOMER};
    std::string createdAt; // ISO datetime
    std::string expiresAt; // ISO datetime
};

/// @brief Wrapper returned by the login endpoint.
struct AuthResult
{
    bool success{false};
    std::string message;            // "ok", "invalid credentials", etc.
    std::optional<Session> session; // present only on success
    std::string token;              // JWT token for authenticated requests
};

// ========================= 1. Users =========================

/// @brief Payload for creating a new customer account.
struct CustomerCreate
{
    std::string fullName;
    std::string email;
    std::string phone;
    std::string passwordHash;
    std::string createdAt; // ISO string
    // Confirm password,phone,createdat should be added -Daimen
};

/// @brief Customer profile sent to frontend.
struct CustomerDTO
{
    UserId userId{};
    std::string fullName;
    std::string email;
    std::string phone;
    std::string createdAt; // ISO datetime
};

/// @brief Partial update payload for a customer profile.
struct CustomerProfileUpdate
{
    UserId userId{};
    std::optional<std::string> fullName;
    std::optional<std::string> email;
    std::optional<std::string> phone;
};

// ========================= 2. Vehicles =========================

/// @brief Payload for registering a new vehicle.
struct VehicleCreate
{
    std::string vin;
    std::string make;
    std::string model;
    int year{0};
    int mileage{0};
};

/// @brief Vehicle data sent to frontend.
struct VehicleDTO
{
    VehicleId vehicleId{};
    UserId ownerId{};
    std::string vin;
    std::string make;
    std::string model;
    int year{0};
    int mileage{0};
};

// ========================= 3. Symptom Forms =========================

/// @brief Payload for creating a new symptom form.
struct SymptomFormCreate
{
    UserId customerId{};
    VehicleId vehicleId{};
    std::string description;
    int severity{0}; // 1-5
};

/// @brief Symptom form data sent to frontend.
struct SymptomFormDTO
{
    SymptomFormId formId{};
    UserId customerId{};
    VehicleId vehicleId{};
    std::string description;
    int severity{0};
    std::string createdAt; // ISO datetime
};

/// @brief Partial update payload for a symptom form.
struct SymptomFormUpdateDTO
{
    SymptomFormId formId{};
    std::optional<std::string> description;
    std::optional<int> severity;
};

// ========================= 4. Mechanics =========================

/// @brief Payload for creating a mechanic profile.
struct MechanicCreate
{
    std::string displayName;
    std::string shopName;
    double hourlyRate{0.0};
    std::vector<std::string> specialties;
};

/// @brief Mechanic profile sent to frontend (includes computed rating).
struct MechanicDTO
{
    MechanicId mechanicId{};
    UserId userId{};
    std::string displayName;
    std::string shopName;
    double hourlyRate{0.0};
    std::vector<std::string> specialties;
    double averageRating{0.0}; // computed from reviews
    int reviewCount{0};        // computed from reviews
};

/// @brief Partial update payload for a mechanic profile.
struct MechanicUpdateDTO
{
    MechanicId mechanicId{};
    std::optional<std::string> displayName;
    std::optional<std::string> shopName;
    std::optional<double> hourlyRate;
    std::optional<std::vector<std::string>> specialties;
};

/// @brief Ranked mechanic match from the discovery engine.
struct MechanicMatch
{
    MechanicId mechanicId{};
    double matchScore{0.0};
    std::vector<std::string> reasons;
};

/// @brief Price estimate returned for a mechanic + symptom form pair.
struct PriceEstimate
{
    int laborCost{};
    int partsCost{};
    int tax{};
    int total{};
    std::string note;
};

// ========================= 5. Appointments =========================

/// @brief Payload for creating an appointment request.
struct AppointmentCreate
{
    UserId customerId{};
    MechanicId mechanicId{};
    SymptomFormId formId{};
    VehicleId vehicleId{};
    std::string scheduledAt; // ISO datetime
    std::string note;
};

/// @brief Appointment data sent to frontend (enriched with names, vehicle, symptoms).
struct AppointmentDTO
{
    AppointmentId appointmentId{};
    UserId customerId{};
    MechanicId mechanicId{};
    SymptomFormId formId{};
    std::string customerName;
    std::string customerEmail;
    std::string customerPhone;
    std::string mechanicName;
    std::string scheduledAt; // ISO datetime
    AppointmentStatus status{AppointmentStatus::REQUESTED};
    std::string note;
    std::string createdAt; // ISO datetime
    // Flattened vehicle info
    VehicleId vehicleId{};
    std::string vehicleDescription; // e.g. "2020 Toyota Camry"
    // Flattened symptom info
    std::string symptoms;
    int severity{0};
};

// ========================= 6. Jobs =========================

/// @brief Single note entry in a job's activity log.
struct JobNoteDTO
{
    JobNoteId noteId{};
    std::string type; // "update", "blocked", "completion"
    std::string text;
    std::string createdAt; // ISO datetime
};

/// @brief Full job status/detail sent to frontend.
struct JobDTO
{
    JobId jobId{};
    AppointmentId appointmentId{};
    UserId customerId{};
    MechanicId mechanicId{};
    JobStage currentStage{JobStage::RECEIVED};
    int percentComplete{0};
    std::vector<JobNoteDTO> notes; // chronological activity log
    std::string updatedAt;         // ISO datetime
    std::string startedAt;         // ISO datetime
    std::string completedAt;       // ISO datetime
    std::string customerName;
    std::string customerEmail;
    std::string vehicleDescription;
    bool isBlocked{false};
};

// ========================= 7. Reviews =========================

/// @brief Payload for submitting a new review.
struct ReviewCreate
{
    UserId customerId{};
    MechanicId mechanicId{};
    JobId jobId{};
    int rating{0}; // 1-5
    std::string comment;
};

/// @brief Review data sent to frontend.
struct ReviewDTO
{
    ReviewId reviewId{};
    MechanicId mechanicId{};
    UserId customerId{};
    JobId jobId{};
    int rating{0};
    std::string comment;
    std::string createdAt;    // ISO datetime
    std::string customerName; // joined from users table
};
