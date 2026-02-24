#pragma once

#include "DatabaseManager.h"

class ProfitabilityEngine{
    private:
        DatabaseManager& db;
        int defaultHourlyRate;
        double taxRate;
    public:
        ProfitabilityEngine(DatabaseManager& db, int defaiultHourlyRate, double taxRate);
        void seTaxRate(double newTaxRate);
        void setDefaultHourlyRate(int newRate);
};