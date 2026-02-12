# Create the library
add_library(TorqueEngine STATIC
    DatabaseManager.cpp
    DatabaseManager.h
 "Records.h" "DTO.h")

# Link libraries this engine needs
target_link_libraries(TorqueEngine 
    PUBLIC 
    nlohmann_json::nlohmann_json
    SQLiteCpp
)

# Let other parts of the app find the headers easily
target_include_directories(TorqueEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})