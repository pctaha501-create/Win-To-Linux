# Build, install, and remove

The preview shell is installed per user; it does not require root and does not replace the display manager.

## Build and test
On Ubuntu 24.04, install: build-essential, cmake, ninja-build, qt6-base-dev, qt6-declarative-dev, qt6-declarative-dev-tools, qml6-module-qtqml, qml6-module-qtqml-models, qml6-module-qtqml-workerscript, qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-layouts, qml6-module-qtquick-window, and qml6-module-qtquick-dialogs. Then run: sh scripts/build.sh.

## Install for the current user
Run sh scripts/install-user.sh. Launch with ~/.local/bin/win-to-linux. Ensure ~/.local/bin is on PATH to use the short command win-to-linux.

## Uninstall
Run sh scripts/uninstall-user.sh. It removes only the installed executable and desktop launcher. Source, build output, and user settings are left untouched.

## Safety
Do not run these scripts with sudo. The application runs within your current desktop session. This preview installer does not install a compositor or modify login/session managers.
