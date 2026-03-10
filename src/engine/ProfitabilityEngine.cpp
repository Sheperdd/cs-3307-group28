/**
 * @file ProfitabilityEngine.cpp
 * @brief Implementation of ProfitabilityEngine — cost estimation and
 *        profitability calculations.
 */
#include "ProfitabilityEngine.h"

ProfitabilityEngine::ProfitabilityEngine(DatabaseManager& db, int defaultHourlyRate, double taxRate)
    : db(db), defaultHourlyRate(defaultHourlyRate), taxRate(taxRate)
{
}

void ProfitabilityEngine::seTaxRate(double newTaxRate)
{
    taxRate = newTaxRate;
}

void ProfitabilityEngine::setDefaultHourlyRate(int newRate)
{
    defaultHourlyRate = newRate;
}
