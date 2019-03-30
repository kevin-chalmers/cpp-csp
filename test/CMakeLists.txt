add_executable(UnitTests channel-tests.cpp process-tests.cpp test.cpp)
target_link_libraries(UnitTests gtest_main)
add_test(NAME AllUnitTests COMMAND UnitTests)