#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <vector>
#include <optional>

#include <nlohmann/json.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

#include "Records.h"

class DatabaseManager {
private:
    // NOTE: SQLite::Database has no default ctor, so we store it as optional.
    std::optional<SQLite::Database> db_;
    std::string lastError_;
    bool inTx_{ false };

public:
    DatabaseManager();
    explicit DatabaseManager(const std::string& dbPath);
    ~DatabaseManager();

    // ---------------- Core Connection / Lifecycle ----------------
    bool connect(const std::string& dbPath);
    void disconnect();
    bool isOpen() const;
    bool runMigrations();
    std::string getLastError() const;

    // ---------------- Transactions ----------------
    void beginTransaction();
    void commit();
    void rollback();
    bool inTransaction() const;

    // ---------------- Legacy JSON API (keep for server/tests) ----------------
    bool addUser(const std::string& name, const std::string& email, const std::string& password);
    bool addMechanic(const std::string& name, const std::string& email, const std::string& password);

    nlohmann::json getUserById(int userId);
    nlohmann::json getUserByEmail(const std::string& userEmail);
    nlohmann::json getAllUsers();

    bool updateUser(int userId, const std::string& name, const std::string& password);
    bool updatePassword(int userId, const std::string& name, const std::string& newPassword);
    bool deleteUser(int userId);

    bool verifyLogin(const std::string& email, const std::string& pasword);
    bool emailExists(const std::string& email);
    int getUserCount();
    void resetDatabase();

    nlohmann::json getMechanicById(int userId);
    nlohmann::json getMechanicByEmail(const std::string& userEmail);
    nlohmann::json getAllMechanics();

    // ---------------- Record-oriented API (Engine core) ----------------

    // Users
    std::optional<UserRecord> getUserByUsername(const std::string& username);
    UserId createUser(const UserRecord& user);
    bool updateUser(UserId userId, const UserUpdate& updates);
    bool updatePasswordHash(UserId userId, const std::string& newHash);
    bool deleteUser(UserId userId);

    // Vehicles
    VehicleId createVehicle(UserId ownerUserId, const VehicleRecord& vehicle);
    std::optional<VehicleRecord> getVehicleById(VehicleId vehicleId);
    std::vector<VehicleRecord> listVehiclesForUser(UserId ownerUserId);
    bool updateVehicle(VehicleId vehicleId, const VehicleUpdate& updates);
    bool deleteVehicle(VehicleId vehicleId);

    // Symptom Forms
    SymptomFormId createSymptomForm(UserId customerId, VehicleId vehicleId, const SymptomFormRecord& form);
    std::optional<SymptomFormRecord> getSymptomFormById(SymptomFormId formId);
    std::vector<SymptomFormRecord> listSymptomFormsForCustomer(UserId customerId);
    bool updateSymptomForm(SymptomFormId formId, const SymptomFormUpdate& updates);
    bool deleteSymptomForm(SymptomFormId formId);

    // Mechanics
    std::optional<MechanicRecord> getMechanicByUserId(UserId userId);
    std::vector<MechanicRecord> searchMechanics(const MechanicSearchFilter& filters);
    bool updateMechanicProfile(MechanicId mechanicId, const MechanicUpdate& updates);

    bool setMechanicAvailability(MechanicId mechanicId, const std::vector<TimeSlot>& slots);
    std::vector<TimeSlot> getMechanicAvailability(MechanicId mechanicId, const DateRange& range);

    // Appointments
    AppointmentId createAppointment(const AppointmentRecord& req);
    std::optional<AppointmentRecord> getAppointmentById(AppointmentId appointmentId);
    std::vector<AppointmentRecord> listAppointmentsForCustomer(UserId customerId);
    std::vector<AppointmentRecord> listAppointmentsForMechanic(MechanicId mechanicId);
    bool updateAppointmentStatus(AppointmentId appointmentId, AppointmentStatus status);
    bool cancelAppointment(AppointmentId appointmentId, const std::string& reason);

    // Jobs
    JobId createJobFromAppointment(AppointmentId appointmentId);
    std::optional<JobRecord> getJobById(JobId jobId);
    std::vector<JobRecord> listOpenJobsForMechanic(MechanicId mechanicId);
    bool updateJobStage(JobId jobId, JobStage stage, int percentComplete, const std::string& note);
    bool markJobComplete(JobId jobId, const std::string& completionNote);

    // Reviews
    ReviewId createReview(const ReviewRecord& review);
    std::vector<ReviewRecord> listReviewsForMechanic(MechanicId mechanicId);
    std::vector<ReviewRecord> listReviewsForCustomer(UserId customerId);
    bool deleteReview(ReviewId reviewId);

private:
    // ---------------- Helpers ----------------
    bool ensureConnected() const;

    // Legacy helpers
    nlohmann::json userRecordToJson(const UserRecord& u) const;
    nlohmann::json getAllByRole(UserRole role) const;
};

#endif // DATABASEMANAGER_H
