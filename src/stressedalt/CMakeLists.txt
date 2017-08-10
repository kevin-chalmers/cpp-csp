add_executable(stressed_alt stressedalt.cpp)
target_link_libraries(stressed_alt ${LIB_FILES})
add_executable(busy_stressedalt busy_stressedalt.cpp)
target_link_libraries(busy_stressedalt ${LIB_FILES})