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

# Prefer Boost::system when available, otherwise fall back to header-only Boost.System.
set(BOOST_SYSTEM_TARGET Boost::system)
set(BOOST_SYSTEM_HEADER_ONLY OFF)

if(NOT TARGET Boost::system)
    if(NOT TARGET Boost::headers)
        find_package(Boost CONFIG REQUIRED COMPONENTS headers)
    endif()
    set(BOOST_SYSTEM_TARGET Boost::headers)
    set(BOOST_SYSTEM_HEADER_ONLY ON)
endif()

# Link the Engine (like the Backend Logic), Boost, and Threads to this executable
target_link_libraries(TorqueServer
    PRIVATE
    TorqueEngine        # Database Logic
    ${BOOST_SYSTEM_TARGET} # Networking (Asio + Beast)
    Threads::Threads    # Thread pool for DB operations
)

if(BOOST_SYSTEM_HEADER_ONLY)
    target_compile_definitions(TorqueServer PRIVATE BOOST_ERROR_CODE_HEADER_ONLY BOOST_SYSTEM_NO_DEPRECATED)
endif()
