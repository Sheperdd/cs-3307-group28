#ifndef CUSTOMERSERVICE_H
#define CUSTOMERSERVICE_H

#include <string>
#include <vector>

#include "DatabaseManager.h"
#include "RatingEngine.h"
#include "ProfitabilityEngine.h"
#include "Records.h"
#include "DTO.h"


// ----------- Validator Interface -----------
class CustomerValidator {
public:
    virtual ~CustomerValidator() = default;
    virtual bool validateCustomerProfile(const UserCreate& profile) = 0;
    virtual bool validateVehicle(const VehicleCreate& vehicle) = 0;
    virtual bool validateSymptomForm(const SymptomFormCreate& form) = 0;
    virtual bool validateReview(const ReviewCreate& review) = 0;
};

// ----------- CustomerService Class -----------
class CustomerService {
private:
    // Private members
    DatabaseManager* db;
    RatingEngine& ratingEngine;
    ProfitabilityEngine& profitabilityEngine;
    CustomerValidator& validator;

    // Private methods
    void validateOwnership(UserId customerId, VehicleId vehicleId);
    void validateFormOwnership(UserId customerId, SymptomFormId formId);
    bool canReviewJob(UserId customerId, JobId jobId);
    std::vector<MechanicRecord> loadCandidateMechanics(const SymptomFormDTO& form, const MechanicSearchFilter& filters);
    std::vector<MechanicMatch> scoreAndRank(const std::vector<MechanicRecord>& candidates, const SymptomFormDTO& form);

public:
    // Constructor
    CustomerService(DatabaseManager* db, RatingEngine& ratingEngine, 
                    ProfitabilityEngine& profitabilityEngine, CustomerValidator& validator);

    // Customer Profile Management
    UserId createCustomerProfile(const UserCreate& profile);
    UserDTO getCustomerProfile(UserId customerId);
    bool updateCustomerProfile(UserId customerId, const UserProfileUpdate& updates);

    // Vehicle Management
    VehicleId addVehicle(UserId customerId, const VehicleCreate& vehicle);
    std::vector<VehicleDTO> listVehicles(UserId customerId);
    bool updateVehicle(VehicleId vehicleId, const VehicleUpdate& updates);
    bool removeVehicle(VehicleId vehicleId);

    // Symptom Form Management
    SymptomFormId createSymptomForm(UserId customerId, VehicleId vehicleId, const SymptomFormCreate& form);
    SymptomFormDTO getSymptomForm(SymptomFormId formId);
    std::vector<SymptomFormDTO> listSymptomForms(UserId customerId);
    bool updateSymptomForm(SymptomFormId formId, const SymptomFormUpdate& updates);
    bool deleteSymptomForm(SymptomFormId formId);

    // Mechanic Discovery
    std::vector<MechanicMatch> findMatchingMechanics(UserId customerId, SymptomFormId formId);
    MechanicDTO viewMechanicProfile(MechanicId mechanicId);
    PriceEstimate requestEstimate(MechanicId mechanicId, SymptomFormId formId);

    // Appointment Management
    AppointmentId requestAppointment(UserId customerId, MechanicId mechanicId, 
                                     SymptomFormId formId, const std::vector<TimeSlot>& preferredSlots);
    bool confirmAppointment(AppointmentId appointmentId);
    bool cancelAppointment(AppointmentId appointmentId, const std::string& reason);
    std::vector<AppointmentDTO> listAppointments(UserId customerId);
    AppointmentDTO getAppointment(AppointmentId appointmentId);

    // Job Tracking
    JobDTO getJobStatus(JobId jobId);
    void unsubscribeFromJobUpdates(SubscriptionId subscriptionId);

    // Review Management
    ReviewId submitReview(UserId customerId, JobId jobId, const ReviewCreate& review);
    std::vector<ReviewDTO> listMyReviews(UserId customerId);
    bool deleteMyReview(UserId customerId, ReviewId reviewId);
};

#endif // CUSTOMERSERVICE_H
