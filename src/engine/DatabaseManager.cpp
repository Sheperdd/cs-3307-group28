#include "DatabaseManager.h"
#include <nlohmann/json.hpp>

std::string DatabaseManager::testConnection() {
    nlohmann::json j;
    j["status"] = "connected";
    return j.dump();
}