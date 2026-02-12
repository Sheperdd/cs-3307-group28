#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <vector>
#include <optional>

#include <nlohmann/json.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

#include "Records.h"

class DatabaseManager
{
private:
    mutable SQLite::Database db;

public:
    DatabaseManager();
    ~DatabaseManager();

    // ---------------- User Management (Legacy JSON API) ----------------
    bool addUser(const std::string &name, const std::string &email, const std::string &password);
    bool addMechanic(const std::string &name, const std::string &email, const std::string &password);

    nlohmann::json getUserById(int userId);
    nlohmann::json getUserByEmail(const std::string &userEmail);
    nlohmann::json getAllUsers();

    bool updateUser(int userId, const std::string &name, const std::string &password);
    bool updatePassword(int userId, const std::string &name, const std::string &newPassword);
    bool deleteUser(int userId);

    bool verifyLogin(const std::string &email, const std::string &password);
    bool emailExists(const std::string &email);
    int getUserCount();
    void resetDatabase();

    nlohmann::json getMechanicById(int userId);
    nlohmann::json getMechanicByEmail(const std::string &userEmail);
    nlohmann::json getAllMechanics();

    // ---------------- Record-oriented API (New Core) ----------------
    UserId createUser(const std::string &name,
                      const std::string &email,
                      const std::string &passwordHash,
                      UserRole role);

    std::optional<UserRecord> getUserRecordById(UserId id);
    std::optional<UserRecord> getUserRecordByEmail(const std::string &email);
    bool updateUserRecord(UserId id, const UserUpdate &update);

private:
    // ---------------- Helpers ----------------
    nlohmann::json userRecordToJson(const UserRecord &u) const;
    nlohmann::json getAllByRole(UserRole role) const;
};

#endif // DATABASEMANAGER_H
