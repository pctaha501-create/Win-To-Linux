# Architecture

1. Linux host: distribution, kernel, session, devices, native applications.
2. Core library: host discovery, capability detection, and safe package command plans.
3. Preview shell: Qt Quick navigation, overview, application discovery/launch, host information, appearance.
4. Future services: process monitoring, settings persistence, package transactions, session management.
5. Compatibility layer: Wine/Bottles integration with per-app compatibility reporting.

Win-To-Linux is not a new kernel and must not hide the host distribution's native tools.

## Distribution discovery
The detector parses key/value lines from /etc/os-release without sourcing or executing the file. It recognizes Debian-family, Red Hat-family, Arch-family, and openSUSE systems. Unknown systems remain generic. Package-manager discovery checks executable files on PATH.

## Package command planning
The planner returns a program and argument list but does not run commands. Package names are validated and not interpolated into shell strings. Privileged operations are intentionally not wired into the current GUI.

## Application discovery and launch
The catalog reads desktop entries from standard user/system application folders and ignores Hidden/NoDisplay entries. Launching validates the path, parses Exec, resolves the executable, and invokes it through QProcess with an argument list. Some desktop-entry wrappers and field-code features are intentionally unsupported.

## Session safety
The preview shell runs inside the current desktop. It must not replace or disable the display manager. A future session launcher must use a unique runtime socket and verify compositor process health and a real Wayland connection before launching the shell.
