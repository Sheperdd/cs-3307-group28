#include "DatabaseManager.h"
#include <iostream>
#include <chrono>
#include <ctime>

//Generates current date and time;
static std::string nowISO() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::gmtime(&t);
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string(buffer);
}

// -------------------- Constructor / Destructor --------------------
DatabaseManager::DatabaseManager() : db("torquedesk.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    try {
        db.exec("CREATE TABLE IF NOT EXISTS users ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "email TEXT, "
                "password TEXT, "
                "role INTEGER, "
                "createdAt TEXT)");
    }
    catch (std::exception& e) {
        std::cerr << "DB initializing Error: " << e.what() << std::endl;
    }
}

DatabaseManager::~DatabaseManager() {}

// -------------------- User CRUD --------------------
UserId DatabaseManager::createUser(const std::string& name,
                                   const std::string& email,
                                   const std::string& passwordHash,
                                   UserRole role)
{
    try {
        SQLite::Statement query(db,
            "INSERT INTO users (email, password, role, createdAt) VALUES (?, ?, ?, ?)");

        query.bind(1, email);
        query.bind(2, passwordHash);
        query.bind(3, static_cast<int>(role));
        query.bind(4, nowISO());

        query.exec();
        return static_cast<UserId>(db.getLastInsertRowid());
    } catch (std::exception& e) {
        std::cerr << "Create User Error: " << e.what() << std::endl;
        return -1;
    }
}

std::optional<UserRecord> DatabaseManager::getUserRecordById(UserId id)
{
    try {
        SQLite::Statement query(db, "SELECT id, email, password, role, createdAt FROM users WHERE id = ?");
        query.bind(1, id);

        if (query.executeStep()) {
            UserRecord u;
            u.id = query.getColumn(0).getInt64();
            u.email = query.getColumn(1).getText();
            u.passwordHash = query.getColumn(2).getText();
            u.role = static_cast<UserRole>(query.getColumn(3).getInt());
            u.createdAt = query.getColumn(4).getText();
            return u;
        }
        return std::nullopt;
    } catch (std::exception& e) {
        std::cerr << "Get User Record Error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<UserRecord> DatabaseManager::getUserRecordByEmail(const std::string& email)
{
    try {
        SQLite::Statement query(db, "SELECT id, email, password, role, createdAt FROM users WHERE email = ?");
        query.bind(1, email);

        if (query.executeStep()) {
            UserRecord u;
            u.id = query.getColumn(0).getInt64();
            u.email = query.getColumn(1).getText();
            u.passwordHash = query.getColumn(2).getText();
            u.role = static_cast<UserRole>(query.getColumn(3).getInt());
            u.createdAt = query.getColumn(4).getText();
            return u;
        }
        return std::nullopt;
    } catch (std::exception& e) {
        std::cerr << "Get User Record by Email Error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool DatabaseManager::updateUserRecord(UserId id, const UserUpdate& update)
{
    try {
        std::string sql = "UPDATE users SET ";
        bool first = true;
        if (update.email.has_value()) {
            sql += "email = ?";
            first = false;
        }
        if (update.role.has_value()) {
            if (!first) sql += ", ";
            sql += "role = ?";
        }
        sql += " WHERE id = ?";

        SQLite::Statement query(db, sql);

        int idx = 1;
        if (update.email.has_value()) query.bind(idx++, *update.email);
        if (update.role.has_value()) query.bind(idx++, static_cast<int>(*update.role));
        query.bind(idx, id);

        query.exec();
        return true;
    } catch (std::exception& e) {
        std::cerr << "Update User Record Error: " << e.what() << std::endl;
        return false;
    }
}

// -------------------- Mechanic helpers --------------------
nlohmann::json DatabaseManager::userRecordToJson(const UserRecord& u) const
{
    return nlohmann::json{
        {"id", u.id},
        {"email", u.email},
        {"role", static_cast<int>(u.role)},
        {"createdAt", u.createdAt}
    };
}

nlohmann::json DatabaseManager::getAllByRole(UserRole role) const
{
    nlohmann::json arr = nlohmann::json::array();
    try {
        SQLite::Statement query(db, "SELECT id, email, password, role, createdAt FROM users WHERE role = ?");
        query.bind(1, static_cast<int>(role));

        while (query.executeStep()) {
            UserRecord u;
            u.id = query.getColumn(0).getInt64();
            u.email = query.getColumn(1).getText();
            u.passwordHash = query.getColumn(2).getText();
            u.role = static_cast<UserRole>(query.getColumn(3).getInt());
            u.createdAt = query.getColumn(4).getText();
            arr.push_back(userRecordToJson(u));
        }
    } catch (std::exception& e) {
        std::cerr << "Get All By Role Error: " << e.what() << std::endl;
    }
    return arr;
}

// -------------------- Convenience wrappers --------------------
bool DatabaseManager::addUser(const std::string& name, const std::string& email, const std::string& password) {
    return createUser(name, email, password, UserRole::CUSTOMER) != -1;
}

bool DatabaseManager::addMechanic(const std::string& name, const std::string& email, const std::string& password) {
    return createUser(name, email, password, UserRole::MECHANIC) != -1;
}

nlohmann::json DatabaseManager::getMechanicById(int userId)
{
    auto rec = getUserRecordById(userId);
    if (!rec.has_value() || rec->role != UserRole::MECHANIC) return {{"error", "Mechanic not found"}};
    return userRecordToJson(*rec);
}

nlohmann::json DatabaseManager::getMechanicByEmail(const std::string& userEmail)
{
    auto rec = getUserRecordByEmail(userEmail);
    if (!rec.has_value() || rec->role != UserRole::MECHANIC) return {{"error", "Mechanic not found"}};
    return userRecordToJson(*rec);
}

nlohmann::json DatabaseManager::getAllMechanics()
{
    return getAllByRole(UserRole::MECHANIC);
}