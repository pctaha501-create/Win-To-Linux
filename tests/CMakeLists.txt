add_executable(wtl-core-test core_test.cpp)
target_link_libraries(wtl-core-test PRIVATE wtl-core Qt6::Core)
add_test(NAME wtl-core-test COMMAND wtl-core-test)
