#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <SQLiteCpp/SQLiteCpp.h>

class DatabaseManager {
private:
	mutable SQLite::Database db;
public:
	DatabaseManager();

	bool addUser(const std::string& name, const std::string& email, const std::string& password);

	int getUserCount();
};

#endif