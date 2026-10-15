# Win-To-Linux

A cross-distribution Linux desktop workspace inspired by Windows 11, built with C++20, Qt 6/QML, CMake, and Ninja.

Current milestone: a Qt Quick preview shell running inside the existing Linux desktop. It does not replace the display manager or claim to be a complete compositor/session.

## Implemented
- Distribution detection from /etc/os-release as data (never sourced or executed).
- Real executable discovery for apt, dnf/yum, pacman, and zypper.
- Safe distro-specific package command planning; the planner does not execute commands.
- Qt Quick shell with Overview, Applications, Linux System, Package Manager, and Settings pages.
- Desktop app discovery from standard XDG folders, search, and launch via QProcess argument lists rather than a shell.
- Dark/light appearance toggle for the current run.
- C++ unit tests and GitHub Actions build/test workflow.

## Ubuntu 24.04 build
    sudo apt update
    sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-declarative-dev qt6-declarative-dev-tools qml6-module-qtquick qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtquick-window
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    cmake --build build --parallel 2
    ctest --test-dir build --output-on-failure
    ./build/src/app/win-to-linux --system
    ./build/src/app/win-to-linux

## Safety
- Unknown distributions stay unknown; no default-to-apt behavior.
- Package command planning never runs package operations.
- Launching apps resolves an executable and uses an argument list, not a shell.
- A desktop entry on disk does not guarantee its app is runnable.
- Do not replace or disable the host display manager during preview testing.

See docs/architecture.md and docs/roadmap.md for architecture and completion gates.
