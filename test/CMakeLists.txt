add_executable(UnitTests channel-tests.cc process-tests.cc test.cc)
target_link_libraries(UnitTests gtest_main)
add_test(NAME AllUnitTests COMMAND UnitTests)