# Find libsodium for password hashing (Argon2id)
find_path(SODIUM_INCLUDE_DIR sodium.h)
find_library(SODIUM_LIBRARY sodium)
if(NOT SODIUM_INCLUDE_DIR OR NOT SODIUM_LIBRARY)
    message(FATAL_ERROR
        "libsodium not found. Install with:\n"
        "  Ubuntu/WSL: sudo apt install libsodium-dev\n"
        "  macOS: brew install libsodium")
endif()

# Create the library
add_library(TorqueEngine STATIC
    DatabaseManager.cpp
    DatabaseManager.h
    CustomerService.cpp
    CustomerService.h
    MechanicService.cpp
    MechanicService.h
    RatingEngine.cpp
    RatingEngine.h
    ProfitabilityEngine.cpp
    ProfitabilityEngine.h
    AuthService.cpp
    AuthService.h
    DefaultCustomerValidator.h
    DTOSerialization.h
    "Records.h"
    "RecordsSerialization.h"
    "DTO.h")

# Link libraries this engine needs
target_link_libraries(TorqueEngine
    PUBLIC
    nlohmann_json::nlohmann_json
    SQLiteCpp
    ${SODIUM_LIBRARY}
)

# Let other parts of the app find the headers easily
target_include_directories(TorqueEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${SODIUM_INCLUDE_DIR})
