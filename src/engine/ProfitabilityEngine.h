/**
 * @file ProfitabilityEngine.h
 * @brief Calculates cost estimates and profit metrics for jobs.
 */
#pragma once

#include "DatabaseManager.h"

/// @brief Engine for price estimates and profitability calculations.
class ProfitabilityEngine{
    private:
        DatabaseManager& db;       ///< shared DB instance
        int defaultHourlyRate;     ///< fallback labor rate ($/hr)
        double taxRate;            ///< tax multiplier (e.g. 0.13 = 13%)
    public:
        /// @param db shared DB reference
        /// @param defaiultHourlyRate default labor rate in dollars
        /// @param taxRate tax rate as decimal
        ProfitabilityEngine(DatabaseManager& db, int defaiultHourlyRate, double taxRate);

        /// @brief Update the tax rate.
        void seTaxRate(double newTaxRate);

        /// @brief Update the default hourly labor rate.
        void setDefaultHourlyRate(int newRate);
};