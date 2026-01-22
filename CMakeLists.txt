cmake_minimum_required(VERSION 3.20)
project(TorqueDesk VERSION 0.1 
                   LANGUAGES CXX
                   DESCRIPTION "A platform designed to take the headache out of automotive repairs for both the mechanic and the vehicle owner.")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Download libs ---
include(FetchContent)

# Download SQLiteCpp (A nice C++ wrapper for SQLite)
FetchContent_Declare(
    SQLiteCpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG 3.3.3
)

set(SQLITECPP_INTERNAL_SQLITE ON CACHE BOOL "Use SQLiteCPP's built-in SQLite" FORCE)
FetchContent_MakeAvailable(SQLiteCpp)

# Download JSON Library
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(json)

# Download GoogleTest (For testing logic)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

# --- Adding our code ---
add_subdirectory(src/engine)
# add_subdirectory(src/client)  Will uncomment this later when we start using Qt for front end
add_subdirectory(tests)