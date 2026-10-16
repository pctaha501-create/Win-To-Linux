add_library(wtl-core STATIC distro_detector.cpp distro_detector.h package_command_planner.cpp package_command_planner.h session_backend.cpp session_backend.h)
target_link_libraries(wtl-core PUBLIC Qt6::Core)
target_include_directories(wtl-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(wtl-core PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>)
