add_executable(wtl-core-test core_test.cpp)
target_link_libraries(wtl-core-test PRIVATE wtl-core Qt6::Core)
add_test(NAME wtl-core-test COMMAND wtl-core-test)

add_executable(wtl-file-manager-test file_manager_test.cpp ../src/app/file_manager.cpp ../src/app/file_manager.h)
target_link_libraries(wtl-file-manager-test PRIVATE Qt6::Core Qt6::Gui)
add_test(NAME wtl-file-manager-test COMMAND wtl-file-manager-test)

add_executable(wtl-process-monitor-test process_monitor_test.cpp ../src/app/process_monitor.cpp ../src/app/process_monitor.h)
target_link_libraries(wtl-process-monitor-test PRIVATE Qt6::Core)
add_test(NAME wtl-process-monitor-test COMMAND wtl-process-monitor-test)

add_test(NAME wtl-qml-smoke COMMAND win-to-linux --smoke-test)
set_tests_properties(wtl-qml-smoke PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
