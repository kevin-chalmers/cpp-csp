add_executable(UnitTests test.cpp)
target_link_libraries(UnitTests gtest_main)
add_test(NAME AllUnitTests COMMAND UnitTests)