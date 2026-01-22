#include "DatabaseManager.h"
#include <nlohmann/json.hpp>
#include <iostream>

// Main Constructor- Opens "torquedesk.db"
// If it doesn't exist, it creates it (OPEN_CREATE)
DatabaseManager::DatabaseManager() : db("torquedesk.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
	// Initialize the tables immediately using basic SQL commands
    try {
        db.exec("CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT, "
            "email TEXT, "
            "password TEXT)");
    }
    catch (std::exception& e) {
        std::cerr << "DB initializing Error: " << e.what() << std::endl;
    }
}

// Adds a user to the database
bool DatabaseManager::addUser(const std::string& name, const std::string& email, const std::string& password) {
    try {
		// using 'statement' cause its the best way to avoid SQL injection and stuff
        SQLite::Statement query(db, "INSERT INTO users (name, email, password) VALUES (?, ?, ?)");

        // Bind the C++ strings to the ? placeholders
        query.bind(1, name);
        query.bind(2, email);
		query.bind(3, password);

        query.exec(); // Run it
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Add User Error: " << e.what() << std::endl;
        return false;
    }
}

int DatabaseManager::getUserCount() {
    // A shortcut to get a value quick from the db
    return db.execAndGet("SELECT COUNT(*) FROM users").getInt();
}