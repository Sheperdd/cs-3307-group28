add_executable(UnitTests test_engine.cpp)

# Link Engine and GoogleTest
target_link_libraries(UnitTests PRIVATE TorqueEngine GTest::gtest_main)

add_executable(CustomerServiceTests test_customer_service.cpp)
target_link_libraries(CustomerServiceTests PRIVATE TorqueEngine GTest::gtest_main)

add_executable(MechanicServiceTests test_mechanic_service.cpp)
target_link_libraries(MechanicServiceTests PRIVATE TorqueEngine GTest::gtest_main)

add_executable(AuthServiceTests test_auth_service.cpp)
target_link_libraries(AuthServiceTests PRIVATE TorqueEngine GTest::gtest_main)

include(CTest)
include(GoogleTest)
gtest_discover_tests(UnitTests)
gtest_discover_tests(CustomerServiceTests)
gtest_discover_tests(MechanicServiceTests)
gtest_discover_tests(AuthServiceTests)
