add_executable(win-to-linux main.cpp)
target_link_libraries(win-to-linux PRIVATE wtl-core Qt6::Core)
