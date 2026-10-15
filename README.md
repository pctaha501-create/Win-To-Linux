# Win-To-Linux

A cross-distribution Linux desktop workspace inspired by Windows 11, built with C++20, Qt 6/QML, CMake, and Ninja.

Current milestone: a Qt Quick preview shell running inside the existing Linux desktop. It does not replace the display manager or claim to be a complete compositor/session.

## Implemented
- Distribution detection from /etc/os-release without sourcing or executing the file.
- Executable discovery for apt, dnf/yum, pacman, and zypper.
- Safe distro-specific package command planning; the planner does not execute commands.
- Qt Quick Overview, Applications, Linux System, Package Manager, Settings, Processes, and Windows Apps pages.
- XDG desktop-entry discovery/search and launch via QProcess argument lists.
- Persistent light/dark preference.
- procfs process list; confirmed SIGTERM requests limited to same-user processes with PID start-time revalidation.
- Wine/Bottles detection; confirmed launch of selected .exe/.msi files through Wine if Wine exists.
- C++ unit tests, a headless QML smoke-test target, and GitHub Actions CI.

## Ubuntu 24.04 build
    sudo apt update
    sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-declarative-dev qt6-declarative-dev-tools qml6-module-qtquick qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtquick-window qml6-module-qtquick-dialogs
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    cmake --build build --parallel 2
    ctest --test-dir build --output-on-failure
    ./build/src/app/win-to-linux --system
    ./build/src/app/win-to-linux

## Safety
- Unknown distributions stay unknown; no default-to-apt behavior.
- Package command planning never runs package operations.
- App launches use argument lists rather than shell interpolation.
- Process stop requests require confirmation, are restricted to the current user's processes, and send SIGTERM rather than SIGKILL.
- Windows installers/programs are never auto-installed; Wine must already be present and each launch requires confirmation.
- A desktop entry on disk does not guarantee its app is runnable.
- Do not replace or disable the host display manager during preview testing.

See docs/architecture.md and docs/roadmap.md.
