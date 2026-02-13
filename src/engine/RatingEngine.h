#include "DatabaseManager.h"
#include "Records.h"

using MechanicID = int64_t;

class RatingEngine {
    private:
        DatabaseManager& db;
        int recencyHalfLifeDays;
    public:
        RatingEngine(DatabaseManager& db, int recencyHalfLifeDays);
        int compute(MechanicID id);
        double getechanicScore(MechanicID id);

        

};