/**
 * @file RatingEngine.h
 * @brief Computes mechanic ratings from reviews with recency weighting.
 */
#pragma once

#include "DatabaseManager.h"
#include "Records.h"

using MechanicID = int64_t;

/// @brief Calculates mechanic scores using review data and time-decay weighting.
class RatingEngine {
    private:
        DatabaseManager& db;        ///< shared DB instance
        int recencyHalfLifeDays;    ///< half-life for review recency decay
    public:
        /// @param db shared DB reference
        /// @param recencyHalfLifeDays days until a review's weight halves
        RatingEngine(DatabaseManager& db, int recencyHalfLifeDays);

        /// @brief Compute raw rating score for a mechanic.
        /// @param id mechanic identifier
        /// @return integer rating score
        int compute(MechanicID id);

        /// @brief Get normalized score (0.0–5.0) for a mechanic.
        /// @param id mechanic identifier
        /// @return weighted average score
        double getechanicScore(MechanicID id);
};