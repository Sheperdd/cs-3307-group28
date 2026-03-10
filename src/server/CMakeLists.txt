# --- Boost (platform-agnostic) ---
# Prefer Boost's CMake package config (common with vcpkg/Windows and many distros).
# CMake 3.30+ removed `FindBoost`, so we avoid module-mode discovery here.
# Users can override discovery with -DBOOST_ROOT=... or -DBoost_ROOT=...
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost CONFIG REQUIRED)
find_package(Threads REQUIRED)

message(STATUS "Boost version: ${Boost_VERSION}")
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")

add_executable(TorqueServer
    main.cpp
    Server.cpp
    HttpSession.cpp
    endpoints/Customers.cpp
    endpoints/Appointments.cpp
    endpoints/Jobs.cpp
    endpoints/Mechanics.cpp
    endpoints/Reviews.cpp
    endpoints/Symptoms.cpp
    endpoints/Vehicles.cpp
)

# Link the Engine (like the Backend Logic), Boost, and Threads to this executable
target_link_libraries(TorqueServer
    PRIVATE
    TorqueEngine        # Database Logic
    Boost::headers      # Networking (Asio + Beast)
    Threads::Threads    # Thread pool for DB operations
    jwt-cpp::jwt-cpp    # JWT token generation/verification
)
