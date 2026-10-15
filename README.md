# Win-To-Linux

A cross-distribution Linux desktop workspace inspired by Windows 11, built with C++20, Qt 6/QML, CMake, and Ninja.

Current milestone: a Qt Quick preview shell running inside the existing Linux desktop. It does not replace the display manager or claim to be a complete compositor/session.

## Implemented
- Distribution detection and safe distro-specific package command planning.
- Qt Quick Overview, Applications, Linux System, Package Manager, Settings, Processes, Windows Apps, and Files pages.
- Session diagnostics: session type, desktop environment, Wayland/X11 display variables, XDG runtime directory ownership/permissions, and known compositor executables on PATH.
- Windowed and optional full-screen launch modes (win-to-linux and win-to-linux --fullscreen).
- Experimental nested Wayland preview launcher (`sh scripts/start-isolated-preview.sh`) that requires an existing user-owned Wayland socket and refuses to change the display manager or start on a physical display.
- File and directory browsing with create-folder, rename, delete, copy, and move actions. Recursive directory deletion requires explicit confirmation; copy/move reject existing destinations and symbolic links, and source operations are restricted to direct children of the current directory.
- File opening is delegated to the host desktop. Copy/move supports regular files and directory trees, but skips symbolic links and special files.
- XDG desktop-entry parsing with dedicated unit tests, hidden/NoDisplay filtering, canonical-path allowlisting, and launch via QProcess argument lists.
- Host-native tool inventory based only on executables actually found on PATH, with extra checks for common Kali security tools.
- Persistent light/dark preference and same-user process inspection/confirmed SIGTERM.
- Asynchronous package operations with bounded output, timeout/cancellation, PolicyKit through pkexec, and explicit install/remove confirmation.
- Wine/Bottles detection and confirmed launch of selected .exe/.msi files through Wine if installed.
- Per-user install/uninstall scripts, desktop launcher, C++ unit tests, headless QML smoke test, install-staging checks, and GitHub Actions CI.

## Build and test on Ubuntu 24.04
Install build-essential, cmake, ninja-build, qt6-base-dev, qt6-declarative-dev, qt6-declarative-dev-tools, qml6-module-qtqml, qml6-module-qtqml-models, qml6-module-qtqml-workerscript, qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-templates, qml6-module-qtquick-layouts, qml6-module-qtquick-window, and qml6-module-qtquick-dialogs. Then run: sh scripts/build.sh

## Install for the current user
    sh scripts/install-user.sh
    ~/.local/bin/win-to-linux
    ~/.local/bin/win-to-linux --fullscreen
    ~/.local/bin/win-to-linux-isolated-preview

## Uninstall
    sh scripts/uninstall-user.sh

## Safety
- Unknown distributions stay unknown; no default-to-apt behavior.
- Package names are validated and arguments are never interpolated into shell strings.
- Install/remove actions require confirmation and PolicyKit authentication.
- Package operations have timeout, cancellation, bounded output, and exit-code reporting.
- Process stop requests require confirmation, are restricted to the current user's processes, and send SIGTERM rather than SIGKILL.
- Windows installers/programs are never auto-installed; Wine must already be present and each launch requires confirmation.
- Native tools are shown only when their executables are detected; no tool is assumed installed.
- File-manager operations validate names and current-directory boundaries; recursive directory deletion requires explicit confirmation. Copy/move does not overwrite existing destinations and refuses symbolic links/special files.
- Session diagnostics are read-only and never replace or disable the host display manager.

See docs/architecture.md, docs/installation.md, and docs/roadmap.md.
