cmake_minimum_required(VERSION 3.20)
project(TorqueDesk VERSION 0.1
                   LANGUAGES CXX
                   DESCRIPTION "A platform designed to take the headache out of automotive repairs for both the mechanic and the vehicle owner.")

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(nlohmann_json)

# Download GoogleTest (For testing logic)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

# Download jwt-cpp (Header-only JWT library, needs OpenSSL)
find_package(OpenSSL REQUIRED)

FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG v0.7.0
)
set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JWT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(jwt-cpp)

# --- Adding our code ---
add_subdirectory(src/engine)
add_subdirectory(src/client)
add_subdirectory(tests)
add_subdirectory(src/server)
