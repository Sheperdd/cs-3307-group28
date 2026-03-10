/**
 * @file DefaultCustomerValidator.h
 * @brief Production implementation of CustomerValidator with basic rules.
 */
#pragma once

#include "CustomerService.h"

/// @brief Default (production) implementation of the CustomerValidator interface.
class DefaultCustomerValidator : public CustomerValidator
{
public:
    /// @brief Name and email must be non-empty.
    bool validateCustomerProfile(const CustomerCreate& profile) override
    {
        return !profile.fullName.empty() && !profile.email.empty();
    }

    /// @brief VIN, make, model non-empty; year positive.
    bool validateVehicle(const VehicleCreate& vehicle) override
    {
        return !vehicle.vin.empty() && !vehicle.make.empty()
            && !vehicle.model.empty() && vehicle.year > 0;
    }

    /// @brief Description non-empty, severity 1–5.
    bool validateSymptomForm(const SymptomFormCreate& form) override
    {
        return !form.description.empty() && form.severity >= 1 && form.severity <= 5;
    }

    /// @brief Rating must be 1–5.
    bool validateReview(const ReviewCreate& review) override
    {
        return review.rating >= 1 && review.rating <= 5;
    }

    /// @brief All foreign-key IDs must be positive.
    bool validateAppointment(const AppointmentCreate& appointment) override
    {
        return appointment.customerId > 0 && appointment.mechanicId > 0
            && appointment.formId > 0 && appointment.vehicleId > 0;
    }
    

};
