#pragma once

#include "CustomerService.h"

/// @brief Default (production) implementation of the CustomerValidator interface.
///        Add real validation rules here as needed.
class DefaultCustomerValidator : public CustomerValidator
{
public:
    bool validateCustomerProfile(const CustomerCreate& profile) override
    {
        return !profile.fullName.empty() && !profile.email.empty();
    }

    bool validateVehicle(const VehicleCreate& vehicle) override
    {
        return !vehicle.vin.empty() && !vehicle.make.empty()
            && !vehicle.model.empty() && vehicle.year > 0;
    }

    bool validateSymptomForm(const SymptomFormCreate& form) override
    {
        return !form.description.empty() && form.severity >= 1 && form.severity <= 5;
    }

    bool validateReview(const ReviewCreate& review) override
    {
        return review.rating >= 1 && review.rating <= 5;
    }
};
