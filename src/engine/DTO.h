#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

#include "Records.h" // IDs, enums, TimeSlot, DateRange, Record/Update structs

/*
 * DTO.h — Data Transfer Objects (server ↔ frontend)
 *
 * One DTO per table, plus auth helpers. These are the shapes sent over HTTP.
 * DB-layer Record/Update structs live in Records.h — don't duplicate them here.
 *
 * Tables (7):  Customers, Vehicles, SymptomForms, Mechanics, Appointments, Jobs, Reviews
 */

// ========================= Shared ID aliases =========================
using SubscriptionId = int64_t;
using SessionId = int64_t;

// ========================= Auth (server → frontend) =========================
// Returned on login; frontend stores for subsequent authenticated requests.
struct Session
{
    SessionId sessionId{};
    UserId userId{};
    UserRole role{UserRole::CUSTOMER};
    std::string createdAt; // ISO datetime
    std::string expiresAt; // ISO datetime
};

// Wrapper returned by the login endpoint.
struct AuthResult
{
    bool success{false};
    std::string message;            // "ok", "invalid credentials", etc.
    std::optional<Session> session; // present only on success
};

// ========================= 1. Users =========================
// POST /users  — frontend → server
struct CustomerCreate
{
    std::string fullName;
    std::string email;
    std::string phone;
    std::string passwordHash;
    std::string createdAt; // ISO string
    //Confirm password,phone,createdat should be added -Daimen
};

// GET /users, GET /users/{id}  — server → frontend
struct CustomerDTO
{
    UserId userId{};
    std::string fullName;
    std::string email;
    std::string phone;
    std::string createdAt; // ISO datetime
};

// PATCH /users/{id}  — frontend → server (all fields optional)
struct CustomerProfileUpdate
{
    UserId userId{};
    std::optional<std::string> fullName;
    std::optional<std::string> email;
    std::optional<std::string> phone;
};

// ========================= 2. Vehicles =========================
// POST /users/{userId}/vehicles  — frontend → server
struct VehicleCreate
{
    std::string vin;
    std::string make;
    std::string model;
    int year{0};
    int mileage{0};
};

// GET /vehicles/{id}, GET /users/{userId}/vehicles  — server → frontend
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
// POST /users/{userId}/symptoms  — frontend → server
struct SymptomFormCreate
{
    UserId customerId{};
    VehicleId vehicleId{};
    std::string description;
    int severity{0}; // 1-5
};

// GET /symptoms/{id}, GET /users/{userId}/symptoms  — server → frontend
struct SymptomFormDTO
{
    SymptomFormId formId{};
    UserId customerId{};
    VehicleId vehicleId{};
    std::string description;
    int severity{0};
    std::string createdAt; // ISO datetime
};

// ========================= 4. Mechanics =========================
// POST /mechanics  — frontend → server
struct MechanicCreate
{
    std::string displayName;
    std::string shopName;
    double hourlyRate{0.0};
    std::vector<std::string> specialties;
};

// GET /mechanics/{id}, GET /mechanics  — server → frontend
// Also used for discovery results and profile views.
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
// Note: for PATCH use MechanicUpdate from Records.h (same fields, all optional).

// Discovery helper — ranked match result from the matching engine.
struct MechanicMatch
{
    MechanicId mechanicId{};
    double matchScore{0.0};
    std::vector<std::string> reasons;
};

// Estimate returned by GET /mechanics/{id}/estimate?formId=…
struct PriceEstimate
{
    int laborCost{};
    int partsCost{};
    int tax{};
    int total{};
    std::string note;
};

// ========================= 5. Appointments =========================
// GET /appointments/{id}, GET /users/{userId}/appointments,
// GET /mechanics/{mechanicId}/appointments  — server → frontend
// One struct covers list items, detail views, and incoming request views.

struct AppointmentCreate
{
    UserId customerId{};
    MechanicId mechanicId{};
    SymptomFormId formId{};
    VehicleId vehicleId{};
    std::string scheduledAt; // ISO datetime
    std::string note;
};

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
// GET /jobs/{id}, GET /mechanics/{mechanicId}/jobs  — server → frontend
// One struct covers job status, card view, and detail view.
struct JobDTO
{
    JobId jobId{};
    AppointmentId appointmentId{};
    UserId customerId{};
    MechanicId mechanicId{};
    JobStage currentStage{JobStage::RECEIVED};
    int percentComplete{0};
    std::string lastNote;
    std::string updatedAt;   // ISO datetime
    std::string startedAt;   // ISO datetime
    std::string completedAt; // ISO datetime
    std::string completionNote;
    std::string customerName;
    std::string customerEmail;
    std::string vehicleDescription;
    bool isBlocked{false};
};

// ========================= 7. Reviews =========================
// POST /reviews  — frontend → server
struct ReviewCreate
{
    int rating{0}; // 1-5
    std::string comment;
};

// GET /reviews, GET /mechanics/{id}/reviews  — server → frontend
struct ReviewDTO
{
    ReviewId reviewId{};
    MechanicId mechanicId{};
    UserId customerId{};
    int rating{0};
    std::string comment;
    std::string createdAt;    // ISO datetime
    std::string customerName; // joined from users table
};