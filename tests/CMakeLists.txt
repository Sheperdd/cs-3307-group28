add_executable(UnitTests test_engine.cpp)

# Link Engine and GoogleTest
target_link_libraries(UnitTests PRIVATE TorqueEngine GTest::gtest_main)