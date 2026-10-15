# Win-To-Linux

A clean, cross-distribution Linux desktop project inspired by the Windows 11 workflow. This is a new repository; the historical WINUX11-claude-version repository remains untouched.

## Principles
- Detect the actual host distribution and package manager.
- Unknown tools are never shown as available without evidence.
- Use program-plus-argument commands, never shell interpolation.
- System changes require explicit user action and authentication.
- CI results do not prove every graphical session or Windows app works.

## Initial baseline
C++20/Qt 6 core, distro detection for Debian/Ubuntu/Kali, Fedora/RHEL, Arch and openSUSE, package command planning, a read-only CLI diagnostic, tests, and GitHub Actions CI. This is a buildable foundation, not yet a complete desktop environment.

## Ubuntu/Debian build
    sudo apt update
    sudo apt install build-essential cmake ninja-build qt6-base-dev
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build --parallel 2
    ctest --test-dir build --output-on-failure
    ./build/src/app/win-to-linux --system

## Roadmap
- [x] Independent repository and C++20/Qt 6 core
- [x] Distribution and package-manager detection
- [x] Distro-specific package command plans and tests
- [ ] Qt Quick desktop shell, navigation, and taskbar
- [ ] Real System Settings and app/process discovery
- [ ] Safe Wayland session launcher and graphical smoke tests
- [ ] User-confirmed package operations with authentication and errors
- [ ] CI/packaging matrix for Ubuntu, Fedora, Arch, and openSUSE
- [ ] Wine/Bottles compatibility integration

Unchecked items are not represented as complete.
