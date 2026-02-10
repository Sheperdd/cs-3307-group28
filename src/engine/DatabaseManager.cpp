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


//getting user info by using their id
nlohmann::json DatabaseManager::getUserById(int userId){
    try{
    SQLite::Statement query (db, "SELECT * FROM users WHERE id = ?");
    query.bind(1,userId);

    if (query.executeStep()){
        nlohmann::json user;
        user ["id"] = query.getColumn(0).getInt();
        user ["name"] = query.getColumn(1).getText();
        user ["email"] = query.getColumn(2).getText();
        return user;
    }
    return nullptr;
    }
    catch (std::exception& e){
    std::cerr << "Get User Error: " << e.what() << std::endl;
    return nullptr;
    }
}

//getting user info by using their email
nlohmann::json DatabaseManager::getUserByEmail(const std::string& userEmail){
    try{
        SQLite::Statement query(db, "SELECT * FROM users WHERE email = ?");
        query.bind(1, userEmail);

        if (query.executeStep()){
            nlohmann::json user;
            user["id"] = query.getColumn(0).getInt();
            user["name"] = query.getColumn(1).getText();
            user["email"] = query.getColumn(2).getText();
            return user;
        }
        return nullptr;
    }catch (std::exception& e){
        std::cerr << "Get User Error: " << e.what() << std::endl;
        return nullptr;
    }
}

//getting all the users of the platform
nlohmann::json DatabaseManager::getAllUsers(){
    try{
        SQLite::Statement query(db, "SELECT * FROM users");
        nlohmann::json users = nlohmann::json::array();

        while (query.executeStep()){
            nlohmann::json user;
            user["id"] = query.getColumn(0).getInt();
            user["name"] = query.getColumn(1).getText();
            user["email"] = query.getColumn(2).getText();
            users.push_back(user);
        }
        return users;
    }
    catch (std::exception& e){
        std::cerr << "Get All Users Error: " << e.what() << std::endl;
        return nlohmann::json::array();
    }
}

//updating user and their information
bool DatabaseManager::updateUser(int userId, const std::string& name, const std::string& password){
    try{
        SQLite::Statement query(db, "UPDATE users SET name = ?, password = ? WHERE id = ?");
        query.bind(1, name);
        query.bind(2, password);
        query.bind(3, userId);

        query.exec();
        return true;
    }catch (std::exception& e){
        std::cerr << "Update User Error: " << e.what() << std::endl;
        return false;
    }
}

//updating  users password
bool DatabaseManager::updatePassword(int userId, const std::string & name,const std::string newPassword){
    try{

        SQLite::Statement query(db, "UPDATE users SET password =? WHERE id = ?");
        query.bind(1, newPassword);
        query.bind(2, userId);

        query.exec();
        return true;
    } catch (std::exception& e){
        std::cerr <<"Update Password Error: " << e.what() << std::endl;
        return false;
    }
}

//deleting user
bool DatabaseManager::deleteUser(int userId){
    try{
        SQLite::Statement query(db, "DELETE FROM users WHERE id = ?");
        query.bind(1, userId);

        query.exec();
        return true;
    }catch (std::exception& e){
        std::cerr << "Delete User Error: "<< e.what() << std::endl;
        return false;
    }
}

//verifying login of a user
bool DatabaseManager::verifyLogin(const std::string &email, const std::string& pasword){
    try{
        SQLite::Statement query(db, "SELECT password FROM users WHERE email = ?");
        query.bind(1,email);

        if (query.executeStep()){
            std::string storedPassword = query.getColumn(0).getText();
            return storedPassword == pasword;;
        }
        return false;
    }catch (std::exception & e){
        std::cerr << "Verification Error:" << e.what() << std::endl;
        return false;
    }
}

//checking if the email exists as information of someones user
bool DatabaseManager::emailExists(const std::string &email){
    try{
        SQLite::Statement query(db, "SELECT COUNT(*) FROM users WHERE email = ?");
        query.bind(1,email);
        query.executeStep();

        return query.getColumn(0).getInt()>0;;
    }catch (std::exception & e){
        std::cerr << "Email Check Verification Error: " << e.what() << std::endl;
        return false;
    }
}