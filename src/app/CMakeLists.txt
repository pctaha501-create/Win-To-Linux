qt_add_executable(win-to-linux main.cpp system_info.cpp system_info.h process_monitor.cpp process_monitor.h package_operation_controller.cpp package_operation_controller.h file_manager.cpp file_manager.h)
target_link_libraries(win-to-linux PRIVATE wtl-core Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2)
qt_add_resources(win-to-linux qml_resources PREFIX "/" FILES qml/Main.qml qml/PackageManagerPage.qml qml/FileManagerPage.qml)
