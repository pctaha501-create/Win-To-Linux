add_executable(wtl-core-test core_test.cpp)
target_link_libraries(wtl-core-test PRIVATE wtl-core Qt6::Core)
add_test(NAME wtl-core-test COMMAND wtl-core-test)

add_executable(wtl-distro-detector-test distro_detector_test.cpp ../src/core/distro_detector.cpp)
target_include_directories(wtl-distro-detector-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/core)
target_link_libraries(wtl-distro-detector-test PRIVATE Qt6::Core)
add_test(NAME wtl-distro-detector-test COMMAND wtl-distro-detector-test)

add_executable(wtl-package-planner-test package_command_planner_test.cpp)
target_link_libraries(wtl-package-planner-test PRIVATE wtl-core Qt6::Core)
add_test(NAME wtl-package-planner-test COMMAND wtl-package-planner-test)

add_executable(wtl-file-manager-test file_manager_test.cpp ../src/app/file_manager.cpp ../src/app/file_manager.h)
target_include_directories(wtl-file-manager-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/app)
target_link_libraries(wtl-file-manager-test PRIVATE Qt6::Core Qt6::Gui)
add_test(NAME wtl-file-manager-test COMMAND wtl-file-manager-test)

add_executable(wtl-process-monitor-test process_monitor_test.cpp ../src/app/process_monitor.cpp ../src/app/process_monitor.h)
target_include_directories(wtl-process-monitor-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/app)
target_link_libraries(wtl-process-monitor-test PRIVATE Qt6::Core)
add_test(NAME wtl-process-monitor-test COMMAND wtl-process-monitor-test)

add_executable(wtl-package-controller-test package_operation_controller_test.cpp ../src/app/package_operation_controller.cpp ../src/app/package_operation_controller.h)
target_include_directories(wtl-package-controller-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/app)
target_link_libraries(wtl-package-controller-test PRIVATE wtl-core Qt6::Core)
add_test(NAME wtl-package-controller-test COMMAND wtl-package-controller-test)

add_test(NAME wtl-qml-smoke COMMAND win-to-linux --smoke-test)
set_tests_properties(wtl-qml-smoke PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_executable(wtl-desktop-entry-test desktop_entry_parser_test.cpp ../src/app/desktop_entry_parser.cpp ../src/app/desktop_entry_parser.h)
target_include_directories(wtl-desktop-entry-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/app)
target_link_libraries(wtl-desktop-entry-test PRIVATE Qt6::Core)
add_test(NAME wtl-desktop-entry-test COMMAND wtl-desktop-entry-test)

add_executable(wtl-system-info-launch-test system_info_launch_test.cpp ../src/app/system_info.cpp ../src/app/system_info.h ../src/app/desktop_entry_parser.cpp ../src/app/desktop_entry_parser.h)
target_include_directories(wtl-system-info-launch-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/app ${CMAKE_CURRENT_SOURCE_DIR}/../src/core)
target_link_libraries(wtl-system-info-launch-test PRIVATE wtl-core Qt6::Core Qt6::Gui)
add_test(NAME wtl-system-info-launch-test COMMAND wtl-system-info-launch-test)
set_tests_properties(wtl-system-info-launch-test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_executable(wtl-screen-share-controller-test screen_share_controller_test.cpp ../src/app/screen_share_controller.cpp ../src/app/screen_share_controller.h)
target_include_directories(wtl-screen-share-controller-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/app)
target_link_libraries(wtl-screen-share-controller-test PRIVATE Qt6::Core Qt6::Gui Qt6::Network)
add_test(NAME wtl-screen-share-controller-test COMMAND wtl-screen-share-controller-test)

add_executable(wtl-wine-prefix-controller-test wine_prefix_controller_test.cpp ../src/app/wine_prefix_controller.cpp ../src/app/wine_prefix_controller.h)
target_include_directories(wtl-wine-prefix-controller-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/app)
target_link_libraries(wtl-wine-prefix-controller-test PRIVATE Qt6::Core)
add_test(NAME wtl-wine-prefix-controller-test COMMAND wtl-wine-prefix-controller-test)

add_executable(wtl-session-backend-test session_backend_test.cpp)
target_link_libraries(wtl-session-backend-test PRIVATE wtl-core Qt6::Core)
add_test(NAME wtl-session-backend-test COMMAND wtl-session-backend-test)
