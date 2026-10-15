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
- Per-user install/uninstall scripts and a desktop launcher.
- C++ unit tests, a headless QML smoke-test target, install-staging checks, and GitHub Actions CI.

## Build and test on Ubuntu 24.04
Install build-essential, cmake, ninja-build, qt6-base-dev, qt6-declarative-dev, qt6-declarative-dev-tools, qml6-module-qtqml, qml6-module-qtqml-models, qml6-module-qtqml-workerscript, qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-layouts, qml6-module-qtquick-window, and qml6-module-qtquick-dialogs. Then run ./scripts/build.sh.

## Install for the current user
    ./scripts/install-user.sh
    ~/.local/bin/win-to-linux

## Uninstall
    ./scripts/uninstall-user.sh

## Safety
- Unknown distributions stay unknown; no default-to-apt behavior.
- Package command planning never runs package operations.
- App launches use argument lists rather than shell interpolation.
- Process stop requests require confirmation, are restricted to the current user's processes, and send SIGTERM rather than SIGKILL.
- Windows installers/programs are never auto-installed; Wine must already be present and each launch requires confirmation.
- A desktop entry on disk does not guarantee its app is runnable.
- Do not replace or disable the host display manager during preview testing.
- Install and uninstall scripts are per-user; never run them with sudo.

See docs/architecture.md, docs/installation.md, and docs/roadmap.md.
