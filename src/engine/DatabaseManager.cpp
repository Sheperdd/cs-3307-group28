#include "DatabaseManager.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <ctime>
#include <stdexcept>

using json = nlohmann::json;

// Generates current date and time as ISO string
static std::string nowISO()
{
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::gmtime(&t);
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string(buffer);
}

// ---- Mechanics schema helpers ----
static void ensureMechanicsSchema(SQLite::Database& db)
{
    db.exec(
        "CREATE TABLE IF NOT EXISTS mechanics ("
        "mechanicId INTEGER PRIMARY KEY, "
        "userId INTEGER UNIQUE, "
        "displayName TEXT, "
        "shopName TEXT, "
        "hourlyRate REAL, "
        "specialties TEXT)"
    );
}

static void ensureMechanicAvailabilitySchema(SQLite::Database& db)
{
    db.exec(
        "CREATE TABLE IF NOT EXISTS mechanic_availability ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "mechanicId INTEGER, "
        "start TEXT, "
        "end TEXT)"
    );
}

static void ensureJobsSchema(SQLite::Database& db)
{
    db.exec(
        "CREATE TABLE IF NOT EXISTS jobs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "appointmentId INTEGER, "
        "mechanicId INTEGER, "
        "customerId INTEGER, "
        "vehicleId INTEGER, "
        "stage INTEGER, "
        "percentComplete INTEGER, "
        "lastNote TEXT, "
        "updatedAt TEXT, "
        "startedAt TEXT, "
        "completedAt TEXT, "
        "completionNote TEXT)"
    );
}

static void ensureAppointmentsSchema(SQLite::Database& db)
{
    db.exec(
        "CREATE TABLE IF NOT EXISTS appointments ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "customerId INTEGER, "
        "mechanicId INTEGER, "
        "vehicleId INTEGER, "
        "symptomFormId INTEGER, "
        "scheduledAt TEXT, "
        "status INTEGER DEFAULT 0, "
        "note TEXT, "
        "createdAt TEXT)"
    );
}

static void ensureReviewsSchema(SQLite::Database& db)
{
    db.exec(
        "CREATE TABLE IF NOT EXISTS reviews ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "jobId INTEGER, "
        "customerId INTEGER, "
        "mechanicId INTEGER, "
        "rating INTEGER, "
        "comment TEXT, "
        "createdAt TEXT)"
    );
}

// ==================== Constructor / Destructor ====================
DatabaseManager::DatabaseManager() : db("torquedesk.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
{
    try
    {
        db.exec("CREATE TABLE IF NOT EXISTS customers ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "email TEXT, "
                "password TEXT, "
                "role INTEGER, "
                "createdAt TEXT)");
    }
    catch (std::exception &e)
    {
        std::cerr << "DB initializing Error: " << e.what() << std::endl;
    }
}

DatabaseManager::~DatabaseManager() {}

// ==================== Utility methods ====================

bool DatabaseManager::deleteUser(UserId userId)
{
    try
    {
        SQLite::Statement query(db, "DELETE FROM customers WHERE id = ?");
        query.bind(1, static_cast<int64_t>(userId));
        query.exec();
        return true;
    }
    catch (std::exception &e)
    {
        std::cerr << "Delete User Error: " << e.what() << std::endl;
        return false;
    }
}

int DatabaseManager::getUserCount()
{
    try
    {
        SQLite::Statement query(db, "SELECT COUNT(*) FROM customers");
        if (query.executeStep())
        {
            return query.getColumn(0).getInt();
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Get User Count Error: " << e.what() << std::endl;
    }
    return 0;
}

// ==================== Record-oriented User CRUD ====================

UserId DatabaseManager::createUser(const std::string &name,
                                   const std::string &email,
                                   const std::string &passwordHash,
                                   UserRole role)
{
    try
    {
        SQLite::Statement query(db,
                                "INSERT INTO customers (email, password, role, createdAt) VALUES (?, ?, ?, ?)");

        query.bind(1, email);
        query.bind(2, passwordHash);
        query.bind(3, static_cast<int>(role));
        query.bind(4, nowISO());

        query.exec();
        return static_cast<UserId>(db.getLastInsertRowid());
    }
    catch (std::exception &e)
    {
        std::cerr << "Create User Error: " << e.what() << std::endl;
        return -1;
    }
}

std::optional<UserRecord> DatabaseManager::getUserRecordById(UserId id)
{
    try
    {
        SQLite::Statement query(db, "SELECT id, email, password, role, createdAt FROM customers WHERE id = ?");
        query.bind(1, id);

        if (query.executeStep())
        {
            UserRecord u;
            u.id = query.getColumn(0).getInt64();
            u.email = query.getColumn(1).getText();
            u.passwordHash = query.getColumn(2).getText();
            u.role = static_cast<UserRole>(query.getColumn(3).getInt());
            u.createdAt = query.getColumn(4).getText();
            return u;
        }
        return std::nullopt;
    }
    catch (std::exception &e)
    {
        std::cerr << "Get User Record Error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<UserRecord> DatabaseManager::getUserRecordByEmail(const std::string &email)
{
    try
    {
        SQLite::Statement query(db, "SELECT id, email, password, role, createdAt FROM customers WHERE email = ?");
        query.bind(1, email);

        if (query.executeStep())
        {
            UserRecord u;
            u.id = query.getColumn(0).getInt64();
            u.email = query.getColumn(1).getText();
            u.passwordHash = query.getColumn(2).getText();
            u.role = static_cast<UserRole>(query.getColumn(3).getInt());
            u.createdAt = query.getColumn(4).getText();
            return u;
        }
        return std::nullopt;
    }
    catch (std::exception &e)
    {
        std::cerr << "Get User Record by Email Error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool DatabaseManager::updateUserRecord(UserId id, const UserUpdate &update)
{
    try
    {
        std::string sql = "UPDATE customers SET ";
        bool first = true;
        if (update.email.has_value())
        {
            sql += "email = ?";
            first = false;
        }
        if (update.role.has_value())
        {
            if (!first)
                sql += ", ";
            sql += "role = ?";
        }
        sql += " WHERE id = ?";

        SQLite::Statement query(db, sql);

        int idx = 1;
        if (update.email.has_value())
            query.bind(idx++, *update.email);
        if (update.role.has_value())
            query.bind(idx++, static_cast<int>(*update.role));
        query.bind(idx, id);

        query.exec();
        return true;
    }
    catch (std::exception &e)
    {
        std::cerr << "Update User Record Error: " << e.what() << std::endl;
        return false;
    }
}

// ==================== Vehicle CRUD ====================

VehicleId DatabaseManager::createVehicle(UserId ownerUserId, const VehicleRecord &vehicle)
{
    if (ownerUserId <= 0) {
        throw std::invalid_argument("createVehicle: invalid ownerUserId");
    }
    if (vehicle.vin.empty()) {
        throw std::invalid_argument("createVehicle: VIN is required");
    }
    if (vehicle.model.empty()) {
        throw std::invalid_argument("createVehicle: model is required");
    }

    try {
        SQLite::Statement stmt(
            db,
            "INSERT INTO vehicles (ownerUserId, vin, make, model, year, createdAt) "
            "VALUES (?, ?, ?, ?, ?, datetime('now'))"
        );

        stmt.bind(1, static_cast<int64_t>(ownerUserId));
        stmt.bind(2, vehicle.vin);
        stmt.bind(3, vehicle.make);
        stmt.bind(4, vehicle.model);

        if (vehicle.year <= 0) stmt.bind(5);
        else stmt.bind(5, vehicle.year);

        stmt.exec();
        return static_cast<VehicleId>(db.getLastInsertRowid());
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("createVehicle failed: ") + e.what());
    }
}

std::optional<VehicleRecord> DatabaseManager::getVehicleById(VehicleId vehicleId)
{
    if (vehicleId <= 0) return std::nullopt;

    try {
        SQLite::Statement stmt(
            db,
            "SELECT id, ownerUserId, vin, make, model, year "
            "FROM vehicles WHERE id = ?"
        );

        stmt.bind(1, static_cast<int64_t>(vehicleId));

        if (!stmt.executeStep()) {
            return std::nullopt;
        }

        VehicleRecord rec{};
        rec.id = static_cast<VehicleId>(stmt.getColumn(0).getInt64());
        rec.ownerUserId = static_cast<UserId>(stmt.getColumn(1).getInt64());
        rec.vin = stmt.getColumn(2).getText();
        rec.make = stmt.getColumn(3).getText();
        rec.model = stmt.getColumn(4).getText();
        rec.year = stmt.getColumn(5).isNull() ? 0 : stmt.getColumn(5).getInt();

        return rec;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("getVehicleById failed: ") + e.what());
    }
}

std::vector<VehicleRecord> DatabaseManager::listVehiclesForUser(UserId ownerUserId)
{
    std::vector<VehicleRecord> results;

    if (ownerUserId <= 0) return results;

    try {
        SQLite::Statement stmt(
            db,
            "SELECT id, ownerUserId, vin, make, model, year "
            "FROM vehicles WHERE ownerUserId = ?"
        );

        stmt.bind(1, static_cast<int64_t>(ownerUserId));

        while (stmt.executeStep()) {
            VehicleRecord rec{};
            rec.id = static_cast<VehicleId>(stmt.getColumn(0).getInt64());
            rec.ownerUserId = static_cast<UserId>(stmt.getColumn(1).getInt64());
            rec.vin = stmt.getColumn(2).getText();
            rec.make = stmt.getColumn(3).getText();
            rec.model = stmt.getColumn(4).getText();
            rec.year = stmt.getColumn(5).isNull() ? 0 : stmt.getColumn(5).getInt();

            results.push_back(std::move(rec));
        }

        return results;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("listVehiclesForUser failed: ") + e.what());
    }
}

bool DatabaseManager::updateVehicle(VehicleId vehicleId, const VehicleUpdate &updates)
{
    if (vehicleId <= 0) return false;

    std::vector<std::string> sets;
    if (updates.make.has_value())  sets.emplace_back("make = ?");
    if (updates.model.has_value()) sets.emplace_back("model = ?");
    if (updates.year.has_value())  sets.emplace_back("year = ?");

    if (sets.empty()) return true; // nothing to update

    std::string sql = "UPDATE vehicles SET ";
    for (size_t i = 0; i < sets.size(); ++i) {
        sql += sets[i];
        if (i + 1 < sets.size()) sql += ", ";
    }
    sql += " WHERE id = ?";

    try {
        SQLite::Statement stmt(db, sql);

        int idx = 1;
        if (updates.make.has_value())  stmt.bind(idx++, updates.make.value());
        if (updates.model.has_value()) stmt.bind(idx++, updates.model.value());
        if (updates.year.has_value())  stmt.bind(idx++, updates.year.value());

        stmt.bind(idx, static_cast<int64_t>(vehicleId));

        return stmt.exec() > 0;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("updateVehicle failed: ") + e.what());
    }
}

bool DatabaseManager::deleteVehicle(VehicleId vehicleId)
{
    if (vehicleId <= 0) return false;

    try {
        SQLite::Statement stmt(
            db,
            "DELETE FROM vehicles WHERE id = ?"
        );

        stmt.bind(1, static_cast<int64_t>(vehicleId));
        return stmt.exec() > 0;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("deleteVehicle failed: ") + e.what());
    }
}

// ==================== Jobs CRUD ====================

JobId DatabaseManager::createJobFromAppointment(AppointmentId appointmentId)
{
    if (appointmentId <= 0) return -1;

    try {
        ensureJobsSchema(db);

        SQLite::Statement stmt(
            db,
            "INSERT INTO jobs (appointmentId, mechanicId, customerId, vehicleId, stage, percentComplete, lastNote, updatedAt, startedAt) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );

        stmt.bind(1, static_cast<int64_t>(appointmentId));
        stmt.bind(2, static_cast<int64_t>(0));
        stmt.bind(3, static_cast<int64_t>(0));
        stmt.bind(4, static_cast<int64_t>(0));
        stmt.bind(5, static_cast<int>(JobStage::RECEIVED));
        stmt.bind(6, 0);
        stmt.bind(7, "");
        stmt.bind(8, nowISO());
        stmt.bind(9, nowISO());

        stmt.exec();
        return static_cast<JobId>(db.getLastInsertRowid());
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("createJobFromAppointment failed: ") + e.what());
    }
}

std::optional<JobRecord> DatabaseManager::getJobById(JobId jobId)
{
    if (jobId <= 0) return std::nullopt;

    try {
        ensureJobsSchema(db);

        SQLite::Statement stmt(
            db,
            "SELECT id, appointmentId, mechanicId, customerId, vehicleId, stage, percentComplete, lastNote, updatedAt, startedAt, completedAt, completionNote "
            "FROM jobs WHERE id = ?"
        );

        stmt.bind(1, static_cast<int64_t>(jobId));

        if (!stmt.executeStep()) {
            return std::nullopt;
        }

        JobRecord rec{};
        rec.id = static_cast<JobId>(stmt.getColumn(0).getInt64());
        rec.appointmentId = static_cast<AppointmentId>(stmt.getColumn(1).getInt64());
        rec.mechanicId = static_cast<MechanicId>(stmt.getColumn(2).getInt64());
        rec.customerId = static_cast<UserId>(stmt.getColumn(3).getInt64());
        rec.vehicleId = static_cast<VehicleId>(stmt.getColumn(4).getInt64());
        rec.stage = static_cast<JobStage>(stmt.getColumn(5).getInt());
        rec.percentComplete = stmt.getColumn(6).getInt();
        rec.lastNote = stmt.getColumn(7).isNull() ? "" : stmt.getColumn(7).getText();
        rec.updatedAt = stmt.getColumn(8).isNull() ? "" : stmt.getColumn(8).getText();
        rec.startedAt = stmt.getColumn(9).isNull() ? "" : stmt.getColumn(9).getText();
        rec.completedAt = stmt.getColumn(10).isNull() ? "" : stmt.getColumn(10).getText();
        rec.completionNote = stmt.getColumn(11).isNull() ? "" : stmt.getColumn(11).getText();

        return rec;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("getJobById failed: ") + e.what());
    }
}

std::vector<JobRecord> DatabaseManager::listOpenJobsForMechanic(MechanicId mechanicId)
{
    std::vector<JobRecord> results;
    if (mechanicId <= 0) return results;

    try {
        ensureJobsSchema(db);

        SQLite::Statement stmt(
            db,
            "SELECT id, appointmentId, mechanicId, customerId, vehicleId, stage, percentComplete, lastNote, updatedAt, startedAt, completedAt, completionNote "
            "FROM jobs WHERE mechanicId = ? AND stage != ?"
        );

        stmt.bind(1, static_cast<int64_t>(mechanicId));
        stmt.bind(2, static_cast<int>(JobStage::DONE));

        while (stmt.executeStep()) {
            JobRecord rec{};
            rec.id = static_cast<JobId>(stmt.getColumn(0).getInt64());
            rec.appointmentId = static_cast<AppointmentId>(stmt.getColumn(1).getInt64());
            rec.mechanicId = static_cast<MechanicId>(stmt.getColumn(2).getInt64());
            rec.customerId = static_cast<UserId>(stmt.getColumn(3).getInt64());
            rec.vehicleId = static_cast<VehicleId>(stmt.getColumn(4).getInt64());
            rec.stage = static_cast<JobStage>(stmt.getColumn(5).getInt());
            rec.percentComplete = stmt.getColumn(6).getInt();
            rec.lastNote = stmt.getColumn(7).isNull() ? "" : stmt.getColumn(7).getText();
            rec.updatedAt = stmt.getColumn(8).isNull() ? "" : stmt.getColumn(8).getText();
            rec.startedAt = stmt.getColumn(9).isNull() ? "" : stmt.getColumn(9).getText();
            rec.completedAt = stmt.getColumn(10).isNull() ? "" : stmt.getColumn(10).getText();
            rec.completionNote = stmt.getColumn(11).isNull() ? "" : stmt.getColumn(11).getText();

            results.push_back(std::move(rec));
        }

        return results;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("listOpenJobsForMechanic failed: ") + e.what());
    }
}

bool DatabaseManager::updateJobStage(JobId jobId, JobStage stage, int percentComplete, const std::string &note)
{
    if (jobId <= 0) return false;

    try {
        ensureJobsSchema(db);

        SQLite::Statement stmt(
            db,
            "UPDATE jobs SET stage = ?, percentComplete = ?, lastNote = ?, updatedAt = ? WHERE id = ?"
        );

        stmt.bind(1, static_cast<int>(stage));
        stmt.bind(2, percentComplete);
        stmt.bind(3, note);
        stmt.bind(4, nowISO());
        stmt.bind(5, static_cast<int64_t>(jobId));

        return stmt.exec() > 0;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("updateJobStage failed: ") + e.what());
    }
}

bool DatabaseManager::markJobComplete(JobId jobId, const std::string &completionNote)
{
    if (jobId <= 0) return false;

    try {
        ensureJobsSchema(db);

        SQLite::Statement stmt(
            db,
            "UPDATE jobs SET stage = ?, percentComplete = ?, completionNote = ?, completedAt = ?, updatedAt = ? WHERE id = ?"
        );

        stmt.bind(1, static_cast<int>(JobStage::DONE));
        stmt.bind(2, 100);
        stmt.bind(3, completionNote);
        stmt.bind(4, nowISO());
        stmt.bind(5, nowISO());
        stmt.bind(6, static_cast<int64_t>(jobId));

        return stmt.exec() > 0;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("markJobComplete failed: ") + e.what());
    }
}

// ==================== Mechanics CRUD ====================

std::optional<MechanicRecord> DatabaseManager::getMechanicByUserId(UserId userId)
{
    if (userId <= 0) return std::nullopt;

    try {
        ensureMechanicsSchema(db);

        SQLite::Statement stmt(
            db,
            "SELECT mechanicId, userId, displayName, shopName, hourlyRate, specialties "
            "FROM mechanics WHERE userId = ?"
        );

        stmt.bind(1, static_cast<int64_t>(userId));

        if (!stmt.executeStep()) {
            return std::nullopt;
        }

        MechanicRecord rec{};
        rec.id = static_cast<MechanicId>(stmt.getColumn(0).getInt64());
        rec.userId = static_cast<UserId>(stmt.getColumn(1).getInt64());
        rec.displayName = stmt.getColumn(2).isNull() ? "" : stmt.getColumn(2).getText();
        rec.shopName = stmt.getColumn(3).isNull() ? "" : stmt.getColumn(3).getText();
        rec.hourlyRate = stmt.getColumn(4).isNull() ? 0.0 : stmt.getColumn(4).getDouble();

        if (!stmt.getColumn(5).isNull()) {
            try {
                auto parsed = json::parse(stmt.getColumn(5).getText());
                rec.specialties = parsed.get<std::vector<std::string>>();
            } catch (...) {
                rec.specialties.clear();
            }
        }

        return rec;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("getMechanicByUserId failed: ") + e.what());
    }
}

std::vector<MechanicRecord> DatabaseManager::searchMechanics(const MechanicSearchFilter &filters)
{
    std::vector<MechanicRecord> results;

    try {
        ensureMechanicsSchema(db);

        std::string sql =
            "SELECT mechanicId, userId, displayName, shopName, hourlyRate, specialties FROM mechanics";

        bool hasWhere = false;
        if (filters.specialty.has_value()) {
            sql += " WHERE specialties LIKE ?";
            hasWhere = true;
        }

        SQLite::Statement stmt(db, sql);

        if (filters.specialty.has_value()) {
            std::string pattern = "%" + filters.specialty.value() + "%";
            stmt.bind(1, pattern);
        }

        while (stmt.executeStep()) {
            MechanicRecord rec{};
            rec.id = static_cast<MechanicId>(stmt.getColumn(0).getInt64());
            rec.userId = static_cast<UserId>(stmt.getColumn(1).getInt64());
            rec.displayName = stmt.getColumn(2).isNull() ? "" : stmt.getColumn(2).getText();
            rec.shopName = stmt.getColumn(3).isNull() ? "" : stmt.getColumn(3).getText();
            rec.hourlyRate = stmt.getColumn(4).isNull() ? 0.0 : stmt.getColumn(4).getDouble();

            if (!stmt.getColumn(5).isNull()) {
                try {
                    auto parsed = json::parse(stmt.getColumn(5).getText());
                    rec.specialties = parsed.get<std::vector<std::string>>();
                } catch (...) {
                    rec.specialties.clear();
                }
            }

            results.push_back(std::move(rec));
        }

        return results;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("searchMechanics failed: ") + e.what());
    }
}

bool DatabaseManager::updateMechanicProfile(MechanicId mechanicId, const MechanicUpdate &updates)
{
    if (mechanicId <= 0) return false;

    try {
        ensureMechanicsSchema(db);

        std::string sql = "UPDATE mechanics SET ";
        bool first = true;

        if (updates.displayName.has_value()) {
            sql += "displayName = ?";
            first = false;
        }
        if (updates.shopName.has_value()) {
            if (!first) sql += ", ";
            sql += "shopName = ?";
            first = false;
        }
        if (updates.hourlyRate.has_value()) {
            if (!first) sql += ", ";
            sql += "hourlyRate = ?";
            first = false;
        }
        if (updates.specialties.has_value()) {
            if (!first) sql += ", ";
            sql += "specialties = ?";
        }

        if (first) {
            return true; // nothing to update
        }

        sql += " WHERE userId = ?";

        SQLite::Statement stmt(db, sql);

        int idx = 1;
        if (updates.displayName.has_value()) stmt.bind(idx++, updates.displayName.value());
        if (updates.shopName.has_value()) stmt.bind(idx++, updates.shopName.value());
        if (updates.hourlyRate.has_value()) stmt.bind(idx++, updates.hourlyRate.value());
        if (updates.specialties.has_value()) {
            stmt.bind(idx++, json(*updates.specialties).dump());
        }
        stmt.bind(idx, static_cast<int64_t>(mechanicId));

        int changed = stmt.exec();
        if (changed > 0) return true;

        // No row updated; insert a new row for this mechanicId/userId
        SQLite::Statement insert(
            db,
            "INSERT OR REPLACE INTO mechanics (mechanicId, userId, displayName, shopName, hourlyRate, specialties) "
            "VALUES (?, ?, ?, ?, ?, ?)"
        );

        insert.bind(1, static_cast<int64_t>(mechanicId));
        insert.bind(2, static_cast<int64_t>(mechanicId));
        insert.bind(3, updates.displayName.value_or(""));
        insert.bind(4, updates.shopName.value_or(""));
        insert.bind(5, updates.hourlyRate.value_or(0.0));
        if (updates.specialties.has_value()) {
            insert.bind(6, json(*updates.specialties).dump());
        } else {
            insert.bind(6);
        }

        return insert.exec() > 0;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("updateMechanicProfile failed: ") + e.what());
    }
}

bool DatabaseManager::setMechanicAvailability(MechanicId mechanicId, const std::vector<TimeSlot> &slots)
{
    if (mechanicId <= 0) return false;

    try {
        ensureMechanicAvailabilitySchema(db);

        db.exec("BEGIN TRANSACTION");

        SQLite::Statement clear(db, "DELETE FROM mechanic_availability WHERE mechanicId = ?");
        clear.bind(1, static_cast<int64_t>(mechanicId));
        clear.exec();

        SQLite::Statement insert(
            db,
            "INSERT INTO mechanic_availability (mechanicId, start, end) VALUES (?, ?, ?)"
        );

        for (const auto& slot : slots) {
            insert.bind(1, static_cast<int64_t>(mechanicId));
            insert.bind(2, slot.start);
            insert.bind(3, slot.end);
            insert.exec();
            insert.reset();
        }

        db.exec("COMMIT");
        return true;
    }
    catch (const SQLite::Exception& e) {
        try { db.exec("ROLLBACK"); } catch (...) {}
        throw std::runtime_error(std::string("setMechanicAvailability failed: ") + e.what());
    }
}

std::vector<TimeSlot> DatabaseManager::getMechanicAvailability(MechanicId mechanicId, const DateRange &range)
{
    std::vector<TimeSlot> results;
    if (mechanicId <= 0) return results;

    try {
        ensureMechanicAvailabilitySchema(db);

        std::string sql = "SELECT start, end FROM mechanic_availability WHERE mechanicId = ?";
        bool filter = !range.start.empty() && !range.end.empty();
        if (filter) {
            sql += " AND start >= ? AND end <= ?";
        }

        SQLite::Statement stmt(db, sql);
        stmt.bind(1, static_cast<int64_t>(mechanicId));
        if (filter) {
            stmt.bind(2, range.start);
            stmt.bind(3, range.end);
        }

        while (stmt.executeStep()) {
            TimeSlot slot;
            slot.start = stmt.getColumn(0).getText();
            slot.end = stmt.getColumn(1).getText();
            results.push_back(std::move(slot));
        }

        return results;
    }
    catch (const SQLite::Exception& e) {
        throw std::runtime_error(std::string("getMechanicAvailability failed: ") + e.what());
    }
}

// ==================== Transactions ====================

void DatabaseManager::beginTransaction()
{
    if (inTx_) return;
    db.exec("BEGIN TRANSACTION");
    inTx_ = true;
}

void DatabaseManager::commit()
{
    if (!inTx_) return;
    db.exec("COMMIT");
    inTx_ = false;
}

void DatabaseManager::rollback()
{
    if (!inTx_) return;
    try { db.exec("ROLLBACK"); } catch (...) {}
    inTx_ = false;
}

// ==================== Appointments ====================

std::optional<AppointmentRecord> DatabaseManager::getAppointmentById(AppointmentId appointmentId)
{
    if (appointmentId <= 0) return std::nullopt;
    try {
        ensureAppointmentsSchema(db);
        SQLite::Statement stmt(db,
            "SELECT id, customerId, mechanicId, vehicleId, symptomFormId, "
            "scheduledAt, status, note, createdAt FROM appointments WHERE id = ?");
        stmt.bind(1, static_cast<int64_t>(appointmentId));
        if (stmt.executeStep()) {
            AppointmentRecord r;
            r.id = stmt.getColumn(0).getInt64();
            r.appointmentId = r.id;
            r.customerId = stmt.getColumn(1).getInt64();
            r.mechanicId = stmt.getColumn(2).getInt64();
            r.vehicleId = stmt.getColumn(3).getInt64();
            r.symptomFormId = stmt.getColumn(4).getInt64();
            r.scheduledAt = stmt.getColumn(5).getText();
            r.status = static_cast<AppointmentStatus>(stmt.getColumn(6).getInt());
            r.note = stmt.getColumn(7).getText();
            r.createdAt = stmt.getColumn(8).getText();
            return r;
        }
        return std::nullopt;
    } catch (const SQLite::Exception &e) {
        throw std::runtime_error(std::string("getAppointmentById failed: ") + e.what());
    }
}

std::vector<AppointmentRecord> DatabaseManager::listAppointmentsForMechanic(MechanicId mechanicId)
{
    std::vector<AppointmentRecord> results;
    if (mechanicId <= 0) return results;
    try {
        ensureAppointmentsSchema(db);
        SQLite::Statement stmt(db,
            "SELECT id, customerId, mechanicId, vehicleId, symptomFormId, "
            "scheduledAt, status, note, createdAt FROM appointments WHERE mechanicId = ?");
        stmt.bind(1, static_cast<int64_t>(mechanicId));
        while (stmt.executeStep()) {
            AppointmentRecord r;
            r.id = stmt.getColumn(0).getInt64();
            r.appointmentId = r.id;
            r.customerId = stmt.getColumn(1).getInt64();
            r.mechanicId = stmt.getColumn(2).getInt64();
            r.vehicleId = stmt.getColumn(3).getInt64();
            r.symptomFormId = stmt.getColumn(4).getInt64();
            r.scheduledAt = stmt.getColumn(5).getText();
            r.status = static_cast<AppointmentStatus>(stmt.getColumn(6).getInt());
            r.note = stmt.getColumn(7).getText();
            r.createdAt = stmt.getColumn(8).getText();
            results.push_back(std::move(r));
        }
        return results;
    } catch (const SQLite::Exception &e) {
        throw std::runtime_error(std::string("listAppointmentsForMechanic failed: ") + e.what());
    }
}

bool DatabaseManager::updateAppointmentStatus(AppointmentId appointmentId, AppointmentStatus status)
{
    if (appointmentId <= 0) return false;
    try {
        ensureAppointmentsSchema(db);
        SQLite::Statement stmt(db,
            "UPDATE appointments SET status = ? WHERE id = ?");
        stmt.bind(1, static_cast<int>(status));
        stmt.bind(2, static_cast<int64_t>(appointmentId));
        return stmt.exec() > 0;
    } catch (const SQLite::Exception &e) {
        throw std::runtime_error(std::string("updateAppointmentStatus failed: ") + e.what());
    }
}

bool DatabaseManager::cancelAppointment(AppointmentId appointmentId, const std::string &reason)
{
    if (appointmentId <= 0) return false;
    try {
        ensureAppointmentsSchema(db);
        SQLite::Statement stmt(db,
            "UPDATE appointments SET status = ?, note = ? WHERE id = ?");
        stmt.bind(1, static_cast<int>(AppointmentStatus::CANCELLED));
        stmt.bind(2, reason);
        stmt.bind(3, static_cast<int64_t>(appointmentId));
        return stmt.exec() > 0;
    } catch (const SQLite::Exception &e) {
        throw std::runtime_error(std::string("cancelAppointment failed: ") + e.what());
    }
}

// ==================== Symptom Forms (lookup) ====================

std::optional<SymptomFormRecord> DatabaseManager::getSymptomFormById(SymptomFormId formId)
{
    // TODO: implement once symptom_forms table schema is finalised
    if (formId <= 0) return std::nullopt;
    return std::nullopt;
}

// ==================== Reviews ====================

std::vector<ReviewRecord> DatabaseManager::listReviewsForMechanic(MechanicId mechanicId)
{
    std::vector<ReviewRecord> results;
    if (mechanicId <= 0) return results;
    try {
        ensureReviewsSchema(db);
        SQLite::Statement stmt(db,
            "SELECT id, jobId, customerId, mechanicId, rating, comment, createdAt "
            "FROM reviews WHERE mechanicId = ?");
        stmt.bind(1, static_cast<int64_t>(mechanicId));
        while (stmt.executeStep()) {
            ReviewRecord r;
            r.id = stmt.getColumn(0).getInt64();
            r.jobId = stmt.getColumn(1).getInt64();
            r.customerId = stmt.getColumn(2).getInt64();
            r.mechanicId = stmt.getColumn(3).getInt64();
            r.rating = stmt.getColumn(4).getInt();
            r.comment = stmt.getColumn(5).getText();
            r.createdAt = stmt.getColumn(6).getText();
            results.push_back(std::move(r));
        }
        return results;
    } catch (const SQLite::Exception &e) {
        throw std::runtime_error(std::string("listReviewsForMechanic failed: ") + e.what());
    }
}

// ==================== Convenience wrappers ====================

bool DatabaseManager::addUser(const std::string &name, const std::string &email, const std::string &password)
{
    return createUser(name, email, password, UserRole::CUSTOMER) != -1;
}

bool DatabaseManager::addMechanic(const std::string &name, const std::string &email, const std::string &password)
{
    return createUser(name, email, password, UserRole::MECHANIC) != -1;
}

bool DatabaseManager::verifyLogin(const std::string &email, const std::string &password)
{
    auto rec = getUserRecordByEmail(email);
    if (!rec.has_value())
        return false;
    return rec->passwordHash == password;
}

bool DatabaseManager::emailExists(const std::string &email)
{
    return getUserRecordByEmail(email).has_value();
}

void DatabaseManager::resetDatabase()
{
    try
    {
        db.exec("DROP TABLE IF EXISTS customers");
        db.exec("CREATE TABLE IF NOT EXISTS customers ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "email TEXT, "
                "password TEXT, "
                "role INTEGER, "
                "createdAt TEXT)");
    }
    catch (std::exception &e)
    {
        std::cerr << "Reset Database Error: " << e.what() << std::endl;
    }
}

bool DatabaseManager::updatePasswordHash(UserId userId, const std::string &newHash)
{
    try
    {
        SQLite::Statement query(db, "UPDATE customers SET password = ? WHERE id = ?");
        query.bind(1, newHash);
        query.bind(2, static_cast<int64_t>(userId));
        return query.exec() > 0;
    }
    catch (std::exception &e)
    {
        std::cerr << "Update Password Error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<UserRecord> DatabaseManager::listAllUsers()
{
    std::vector<UserRecord> result;
    try
    {
        SQLite::Statement query(db, "SELECT id, email, password, role, createdAt FROM customers");
        while (query.executeStep())
        {
            UserRecord u;
            u.id = query.getColumn(0).getInt64();
            u.email = query.getColumn(1).getText();
            u.passwordHash = query.getColumn(2).getText();
            u.role = static_cast<UserRole>(query.getColumn(3).getInt());
            u.createdAt = query.getColumn(4).getText();
            result.push_back(std::move(u));
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "List All Users Error: " << e.what() << std::endl;
    }
    return result;
}

std::vector<UserRecord> DatabaseManager::listUsersByRole(UserRole role)
{
    std::vector<UserRecord> result;
    try
    {
        SQLite::Statement query(db, "SELECT id, email, password, role, createdAt FROM customers WHERE role = ?");
        query.bind(1, static_cast<int>(role));
        while (query.executeStep())
        {
            UserRecord u;
            u.id = query.getColumn(0).getInt64();
            u.email = query.getColumn(1).getText();
            u.passwordHash = query.getColumn(2).getText();
            u.role = static_cast<UserRole>(query.getColumn(3).getInt());
            u.createdAt = query.getColumn(4).getText();
            result.push_back(std::move(u));
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "List Users By Role Error: " << e.what() << std::endl;
    }
    return result;
}

std::vector<MechanicRecord> DatabaseManager::listAllMechanics()
{
    return searchMechanics(MechanicSearchFilter{});
}

std::optional<MechanicRecord> DatabaseManager::getMechanicByEmail(const std::string &email)
{
    auto userOpt = getUserRecordByEmail(email);
    if (!userOpt.has_value() || userOpt->role != UserRole::MECHANIC)
        return std::nullopt;
    return getMechanicByUserId(userOpt->id);
}
