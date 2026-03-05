#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <vector>
#include <optional>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Records.h"

class DatabaseManager
{
private:
    mutable SQLite::Database db;
    std::string lastError_;
    bool inTx_{false};

public:
    DatabaseManager();
    explicit DatabaseManager(const std::string &dbPath);
    ~DatabaseManager();

    // ---------------- Core Connection / Lifecycle ----------------
    bool connect(const std::string &dbPath);
    void disconnect();
    bool isOpen() const;
    bool runMigrations();
    std::string getLastError() const;

    // ---------------- Transactions ----------------
    void beginTransaction();
    void commit();
    void rollback();
    bool inTransaction() const;

    // ---------------- Convenience wrappers ----------------
    bool addUser(const std::string &name, const std::string &email, const std::string &password);
    bool addMechanic(const std::string &name, const std::string &email, const std::string &password);

    int getUserCount();
    bool verifyLogin(const std::string &email, const std::string &password);
    bool emailExists(const std::string &email);
    void resetDatabase();

    // ---------------- Record-oriented API (Engine core) ----------------

    // Users
    std::optional<UserRecord> getUserByUsername(const std::string &username);
    UserId createUser(const UserRecord &user);
    bool updateUser(UserId userId, const UserUpdate &updates);
    bool updatePasswordHash(UserId userId, const std::string &newHash);
    bool deleteUser(UserId userId);

    // Existing user-oriented API used by current engine/server code
    UserId createUser(const std::string &name,
                      const std::string &email,
                      const std::string &passwordHash,
                      UserRole role);

    std::optional<UserRecord> getUserRecordById(UserId id);
    std::optional<UserRecord> getUserRecordByEmail(const std::string &email);
    std::vector<UserRecord> listAllUsers();
    std::vector<UserRecord> listUsersByRole(UserRole role);
    bool updateUserRecord(UserId id, const UserUpdate &update);

    // Vehicles
    VehicleId createVehicle(UserId ownerUserId, const VehicleRecord &vehicle);
    std::optional<VehicleRecord> getVehicleById(VehicleId vehicleId);
    std::vector<VehicleRecord> listVehiclesForUser(UserId ownerUserId);
    bool updateVehicle(VehicleId vehicleId, const VehicleUpdate &updates);
    bool deleteVehicle(VehicleId vehicleId);

    // Symptom Forms
    SymptomFormId DatabaseManager::createSymptomForm(const SymptomFormRecord &form);
    SymptomFormRecord DatabaseManager::getSymptomFormById(SymptomFormId formId);
    std::vector<SymptomFormRecord> DatabaseManager::listSymptomFormsForCustomer(UserId customerId);
    bool DatabaseManager::updateSymptomForm (SymptomFormId formId, const SymptomFormUpdate &updates);
    bool DatabaseManager::deleteSymptomForm(SymptomFormId formId);

    // Mechanics
    std::optional<MechanicRecord> getMechanicByUserId(UserId userId);
    std::optional<MechanicRecord> getMechanicByEmail(const std::string &email);
    std::vector<MechanicRecord> searchMechanics(const MechanicSearchFilter &filters);
    std::vector<MechanicRecord> listAllMechanics();
    bool updateMechanicProfile(MechanicId mechanicId, const MechanicUpdate &updates);

    bool setMechanicAvailability(MechanicId mechanicId, const std::vector<TimeSlot> &slots);
    std::vector<TimeSlot> getMechanicAvailability(MechanicId mechanicId, const DateRange &range);

    // Appointments
    AppointmentId createAppointment(const AppointmentRecord &req);
    std::optional<AppointmentRecord> getAppointmentById(AppointmentId appointmentId);
    std::vector<AppointmentRecord> listAppointmentsForCustomer(UserId customerId);
    std::vector<AppointmentRecord> listAppointmentsForMechanic(MechanicId mechanicId);
    bool updateAppointmentStatus(AppointmentId appointmentId, AppointmentStatus status);
    bool cancelAppointment(AppointmentId appointmentId, const std::string &reason);

    // Jobs
    JobId createJobFromAppointment(AppointmentId appointmentId);
    std::optional<JobRecord> getJobById(JobId jobId);
    std::vector<JobRecord> listOpenJobsForMechanic(MechanicId mechanicId);
    bool updateJobStage(JobId jobId, JobStage stage, int percentComplete, const std::string &note);
    bool markJobComplete(JobId jobId, const std::string &completionNote);

    // Reviews
    ReviewId createReview(const ReviewRecord &review);
    std::vector<ReviewRecord> listReviewsForMechanic(MechanicId mechanicId);
    std::vector<ReviewRecord> listReviewsForCustomer(UserId customerId);
    bool deleteReview(ReviewId reviewId);

private:
    // ---------------- Helpers ----------------
    bool ensureConnected() const;
};

#endif // DATABASEMANAGER_H
