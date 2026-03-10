/**
 * @file DatabaseManager.h
 * @brief SQLite database layer — CRUD for all entities (users, vehicles,
 *        symptom forms, mechanics, appointments, jobs, reviews).
 */
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <vector>
#include <optional>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Records.h"

/// @brief Central data-access object wrapping a single SQLite connection.
///
/// All DB operations go through this class. The server owns one instance
/// shared across sessions; blocking calls are dispatched to a thread pool.
class DatabaseManager
{
private:
    mutable SQLite::Database db;  ///< underlying SQLiteCpp connection
    std::string lastError_;       ///< last error message for diagnostics
    bool inTx_{false};            ///< true while inside a transaction

public:
    /// @brief Open/create the default DB file.
    DatabaseManager();
    /// @brief Open/create a DB at the given path.
    explicit DatabaseManager(const std::string &dbPath);
    ~DatabaseManager();

    // ---------------- Core Connection / Lifecycle ----------------

    /// @brief Open a DB connection to the given path.
    bool connect(const std::string &dbPath);
    /// @brief Close the current connection.
    void disconnect();
    /// @brief Check if a DB connection is open.
    bool isOpen() const;
    /// @brief Create tables / apply schema migrations.
    bool runMigrations();
    /// @brief Get the last recorded error string.
    std::string getLastError() const;

    // ---------------- Transactions ----------------

    /// @brief Begin an explicit transaction.
    void beginTransaction();
    /// @brief Commit the current transaction.
    void commit();
    /// @brief Roll back the current transaction.
    void rollback();
    /// @brief Check if currently inside a transaction.
    bool inTransaction() const;

    // ---------------- Convenience wrappers ----------------

    /// @brief Quick-add a customer user (name, email, password).
    bool addUser(const std::string &name, const std::string &email, const std::string &password);
    /// @brief Quick-add a mechanic user (name, email, password).
    bool addMechanic(const std::string &name, const std::string &email, const std::string &password);
    /// @brief Count total users in the DB.
    int getUserCount();
    /// @brief Verify login credentials (plaintext comparison — legacy).
    bool verifyLogin(const std::string &email, const std::string &password);
    /// @brief Check if an email is already registered.
    bool emailExists(const std::string &email);
    /// @brief Drop and recreate all tables.
    void resetDatabase();

    // ---------------- Record-oriented API (Engine core) ----------------

    // -- Users --

    /// @brief Look up a user by username.
    std::optional<UserRecord> getUserByUsername(const std::string &username);
    /// @brief Insert a new user from a UserRecord.
    UserId createUser(const UserRecord &user);
    /// @brief Partial-update a user row.
    bool updateUser(UserId userId, const UserUpdate &updates);
    /// @brief Overwrite password hash for a user.
    bool updatePasswordHash(UserId userId, const std::string &newHash);
    /// @brief Delete a user by ID.
    bool deleteUser(UserId userId);

    /// @brief Insert a new user with explicit fields (convenience overload).
    UserId createUser(const std::string &name,
                      const std::string &email,
                      const std::string &passwordHash,
                      UserRole role);

    /// @brief Get a user record by primary key.
    std::optional<UserRecord> getUserRecordById(UserId id);
    /// @brief Get a user record by email address.
    std::optional<UserRecord> getUserRecordByEmail(const std::string &email);
    /// @brief List every user in the database.
    std::vector<UserRecord> listAllUsers();
    /// @brief List users filtered by role.
    std::vector<UserRecord> listUsersByRole(UserRole role);
    /// @brief Partial-update a user record.
    bool updateUserRecord(UserId id, const UserUpdate &update);

    // -- Vehicles --

    /// @brief Insert a new vehicle record.
    VehicleId createVehicle(UserId ownerUserId, const VehicleRecord &vehicle);
    /// @brief Get a vehicle by ID.
    std::optional<VehicleRecord> getVehicleById(VehicleId vehicleId);
    /// @brief List all vehicles for a given owner.
    std::vector<VehicleRecord> listVehiclesForUser(UserId ownerUserId);
    /// @brief Partial-update a vehicle row.
    bool updateVehicle(VehicleId vehicleId, const VehicleUpdate &updates);
    /// @brief Delete a vehicle by ID.
    bool deleteVehicle(VehicleId vehicleId);

    // -- Symptom Forms --

    /// @brief Insert a new symptom form.
    SymptomFormId createSymptomForm(const SymptomFormRecord &form);
    /// @brief Fetch a symptom form by ID.
    SymptomFormRecord getSymptomFormById(SymptomFormId formId);
    /// @brief List symptom forms for a customer.
    std::vector<SymptomFormRecord> listSymptomFormsForCustomer(UserId customerId);
    /// @brief Partial-update a symptom form.
    bool updateSymptomForm(SymptomFormId formId, const SymptomFormUpdate &updates);
    /// @brief Delete a symptom form.
    bool deleteSymptomForm(SymptomFormId formId);

    // -- Mechanics --

    /// @brief Get mechanic profile by user ID.
    std::optional<MechanicRecord> getMechanicByUserId(UserId userId);
    /// @brief Get mechanic profile by email.
    std::optional<MechanicRecord> getMechanicByEmail(const std::string &email);
    /// @brief Search mechanics with optional filters.
    std::vector<MechanicRecord> searchMechanics(const MechanicSearchFilter &filters);
    /// @brief List all mechanic profiles.
    std::vector<MechanicRecord> listAllMechanics();
    /// @brief Partial-update a mechanic profile.
    bool updateMechanicProfile(MechanicId mechanicId, const MechanicUpdate &updates);
    /// @brief Set availability time slots for a mechanic.
    bool setMechanicAvailability(MechanicId mechanicId, const std::vector<TimeSlot> &slots);
    /// @brief Get mechanic availability within a date range.
    std::vector<TimeSlot> getMechanicAvailability(MechanicId mechanicId, const DateRange &range);

    // -- Appointments --

    /// @brief Create a new appointment record.
    AppointmentId createAppointment(const AppointmentRecord &req);
    /// @brief Fetch an appointment by ID.
    std::optional<AppointmentRecord> getAppointmentById(AppointmentId appointmentId);
    /// @brief List all appointments for a customer.
    std::vector<AppointmentRecord> listAppointmentsForCustomer(UserId customerId);
    /// @brief List all appointments for a mechanic.
    std::vector<AppointmentRecord> listAppointmentsForMechanic(MechanicId mechanicId);
    /// @brief Update appointment status.
    bool updateAppointmentStatus(AppointmentId appointmentId, AppointmentStatus status);
    /// @brief Update the scheduled time for an appointment.
    bool updateAppointmentScheduledAt(AppointmentId appointmentId, const std::string &newScheduledAt);
    /// @brief Reschedule an appointment with a new time and note.
    bool rescheduleAppointment(AppointmentId appointmentId, const std::string &newScheduledAt, const std::string &note);
    /// @brief Cancel an appointment with a reason.
    bool cancelAppointment(AppointmentId appointmentId, const std::string &reason);

    // -- Jobs --

    /// @brief Create a job record from a confirmed appointment.
    JobId createJobFromAppointment(AppointmentId appointmentId);
    /// @brief Fetch a job by ID.
    std::optional<JobRecord> getJobById(JobId jobId);
    /// @brief List open (non-complete) jobs for a mechanic.
    std::vector<JobRecord> listOpenJobsForMechanic(MechanicId mechanicId);
    /// @brief Update a job's stage and percent-complete.
    bool updateJobStage(JobId jobId, JobStage stage, int percentComplete);
    /// @brief Mark a job as complete (stage=DONE, 100%).
    bool markJobComplete(JobId jobId);

    // -- Job Notes --

    /// @brief Append a note to a job's activity log.
    JobNoteId addJobNote(JobId jobId, const std::string &type, const std::string &text);
    /// @brief List all notes for a job in chronological order.
    std::vector<JobNoteRecord> listJobNotes(JobId jobId);

    // -- Reviews --

    /// @brief Insert a new review record.
    ReviewId createReview(const ReviewRecord &review);
    /// @brief List all reviews for a mechanic.
    std::vector<ReviewRecord> listReviewsForMechanic(MechanicId mechanicId);
    /// @brief List all reviews by a customer.
    std::vector<ReviewRecord> listReviewsForCustomer(UserId customerId);
    /// @brief Delete a review by ID.
    bool deleteReview(ReviewId reviewId);

private:
    /// @brief Verify the DB connection is open before operations.
    bool ensureConnected() const;
};

#endif // DATABASEMANAGER_H
