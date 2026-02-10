#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <nlohmann/adl_serializer.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

class DatabaseManager {
private:
	mutable SQLite::Database db;
public:
	DatabaseManager();

	bool addUser(const std::string& name, const std::string& email, const std::string& password);

	nlohmann::json getUserById(int userId);
	nlohmann::json getUserByEmail(const std::string& userEmail);
	nlohmann::json getAllUsers();
	bool updateUser(int userId, const std::string& name, const std::string& password);
	bool updatePassword(int userId, const std::string & name,const std::string newPassword);
	bool deleteUser(int userId);

	bool verifyLogin(const std::string &email, const std::string& pasword);
	bool emailExists(const std::string &email);

	int getUserCount();
};

#endif