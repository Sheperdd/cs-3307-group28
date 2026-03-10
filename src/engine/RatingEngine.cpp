/**
 * @file RatingEngine.cpp
 * @brief Implementation of RatingEngine — mechanic scoring from reviews
 *        with time-decay weighting.
 */
#include "RatingEngine.h"

RatingEngine::RatingEngine(DatabaseManager& db, int recencyHalfLifeDays)
    : db(db), recencyHalfLifeDays(recencyHalfLifeDays)
{
}

int RatingEngine::compute(MechanicID id)
{
    // TODO: implement proper rating computation
    return 0;
}

double RatingEngine::getechanicScore(MechanicID id)
{
    // TODO: implement proper scoring from reviews
    return 0.0;
}
