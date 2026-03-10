/**
 * @file CustomerService.h
 * @brief Business logic layer for customer operations — profiles, vehicles,
 *        symptom forms, mechanic discovery, appointments, jobs, and reviews.
 */
#ifndef CUSTOMERSERVICE_H
#define CUSTOMERSERVICE_H

#include <string>
#include <vector>

#include "DatabaseManager.h"
#include "RatingEngine.h"
#include "ProfitabilityEngine.h"
#include "Records.h"
#include "DTO.h"

/// @brief Abstract validator interface for customer-facing input validation.
class CustomerValidator
{
public:
    virtual ~CustomerValidator() = default;
    /// @brief Validate a customer profile creation payload.
    virtual bool validateCustomerProfile(const CustomerCreate &profile) = 0;
    /// @brief Validate a vehicle creation payload.
    virtual bool validateVehicle(const VehicleCreate &vehicle) = 0;
    /// @brief Validate a symptom form creation payload.
    virtual bool validateSymptomForm(const SymptomFormCreate &form) = 0;
    /// @brief Validate a review creation payload.
    virtual bool validateReview(const ReviewCreate &review) = 0;
    /// @brief Validate an appointment creation payload.
    virtual bool validateAppointment(const AppointmentCreate &appointment) = 0;
};

/// @brief Service handling all customer-facing business logic.
///
/// Delegates DB access to DatabaseManager, scoring to RatingEngine/ProfitabilityEngine,
/// and input validation to a CustomerValidator implementation.
class CustomerService
{
private:
    DatabaseManager *db;                        ///< shared DB instance
    RatingEngine &ratingEngine;                 ///< mechanic scoring engine
    ProfitabilityEngine &profitabilityEngine;   ///< cost estimation engine
    CustomerValidator &validator;               ///< input validator

    /// @brief Verify customer owns the given vehicle, throw if not.
    void validateOwnership(UserId customerId, VehicleId vehicleId);
    /// @brief Verify customer owns the given symptom form, throw if not.
    void validateFormOwnership(UserId customerId, SymptomFormId formId);
    /// @brief Check if customer is eligible to review a completed job.
    bool canReviewJob(UserId customerId, JobId jobId);
    /// @brief Load mechanics matching a symptom form and optional filters.
    std::vector<MechanicRecord> loadCandidateMechanics(const SymptomFormDTO &form, const MechanicSearchFilter &filters);
    /// @brief Score and sort candidate mechanics by match quality.
    std::vector<MechanicMatch> scoreAndRank(const std::vector<MechanicRecord> &candidates, const SymptomFormDTO &form);

public:
    CustomerService(DatabaseManager *db, RatingEngine &ratingEngine,
                    ProfitabilityEngine &profitabilityEngine, CustomerValidator &validator);

    // ---- Customer Profile ----

    /// @brief Create a new customer account.
    UserId createCustomerProfile(const CustomerCreate &profile);
    /// @brief Fetch a customer's profile by ID.
    CustomerDTO getCustomerProfile(UserId customerId);
    /// @brief Update editable profile fields.
    bool updateCustomerProfile(UserId userId, CustomerProfileUpdate &update);
    /// @brief Permanently delete a customer account.
    bool deleteCustomerProfile(UserId customerId);
    /// @brief Overwrite the stored password hash.
    bool updateCustomerPasswordHash(UserId customerId, const std::string &newHash);

    // ---- Vehicles ----

    /// @brief Register a new vehicle for a customer.
    VehicleId addVehicle(UserId customerId, const VehicleCreate &vehicle);
    /// @brief Get a single vehicle by ID.
    VehicleDTO getVehicle(VehicleId vehicleId);
    /// @brief List all vehicles for a customer.
    std::vector<VehicleDTO> listVehicles(UserId customerId);
    /// @brief Update vehicle fields.
    bool updateVehicle(VehicleId vehicleId, const VehicleUpdate &updates);
    /// @brief Delete a vehicle record.
    bool removeVehicle(VehicleId vehicleId);

    // ---- Symptom Forms ----

    /// @brief Submit a new symptom form for a vehicle.
    SymptomFormId createSymptomForm(const SymptomFormCreate &form);
    /// @brief Fetch a single symptom form.
    SymptomFormDTO getSymptomForm(SymptomFormId formId);
    /// @brief List all symptom forms for a customer.
    std::vector<SymptomFormDTO> listSymptomForms(UserId customerId);
    /// @brief Update symptom form description/severity.
    bool updateSymptomForm(SymptomFormId formId, const SymptomFormUpdate &updates);
    /// @brief Delete a symptom form.
    bool deleteSymptomForm(SymptomFormId formId);

    // ---- Mechanic Discovery ----

    /// @brief Find and rank mechanics suitable for a symptom form.
    std::vector<MechanicMatch> findMatchingMechanics(UserId customerId, SymptomFormId formId);
    /// @brief Get a mechanic's public profile.
    MechanicDTO viewMechanicProfile(MechanicId mechanicId);
    /// @brief Get a price estimate from a mechanic for a symptom form.
    PriceEstimate requestEstimate(MechanicId mechanicId, SymptomFormId formId);

    // ---- Appointments ----

    /// @brief Create a new appointment request.
    AppointmentId requestAppointment(AppointmentCreate appointment);
    /// @brief Confirm a pending appointment.
    bool confirmAppointment(AppointmentId appointmentId);
    /// @brief Cancel an appointment with a reason.
    bool cancelAppointment(AppointmentId appointmentId, const std::string &reason);
    /// @brief List all appointments for a customer.
    std::vector<AppointmentDTO> listAppointments(UserId customerId);
    /// @brief Get a single appointment with enriched details.
    AppointmentDTO getAppointment(AppointmentId appointmentId);
    /// @brief Directly set an appointment's status.
    bool updateAppointmentStatus(AppointmentId appointmentId, AppointmentStatus status);

    // ---- Job Tracking ----

    /// @brief Get current job status and notes.
    JobDTO getJobStatus(JobId jobId);
    /// @brief Unsubscribe from real-time job updates.
    void unsubscribeFromJobUpdates(SubscriptionId subscriptionId);

    // ---- Reviews ----

    /// @brief Submit a review for a completed job.
    ReviewId submitReview(const ReviewCreate &review);
    /// @brief List all reviews written by a customer.
    std::vector<ReviewDTO> listMyReviews(UserId customerId);
    /// @brief Delete a review by ID.
    bool deleteMyReview(ReviewId reviewId);
};

#endif // CUSTOMERSERVICE_H
