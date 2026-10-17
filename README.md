# Win-To-Linux

A cross-distribution Linux desktop workspace inspired by Windows 11, built with C++20, Qt 6/QML, CMake, and Ninja.

Current milestone: a Qt Quick preview shell running inside the existing Linux desktop. It does not replace the display manager or claim to be a complete compositor/session.

## Implemented
- Distribution detection and safe distro-specific package command planning. Application discovery follows XDG_DATA_HOME/XDG_DATA_DIRS and includes per-user/system Flatpak exports plus Snap desktop entries; launch paths are validated against the same discovered trusted roots.
- Qt Quick Overview, Applications, Linux System, Package Manager, Settings, Processes, Windows Apps, and Files pages.
- Session diagnostics: session type, desktop environment, Wayland/X11 display variables, XDG runtime directory ownership/permissions, and known compositor executables on PATH.
- Windowed and optional full-screen launch modes (win-to-linux and win-to-linux --fullscreen).
- Experimental nested Wayland preview launcher (`sh scripts/start-isolated-preview.sh`) that requires an existing user-owned Wayland socket and refuses to change the display manager or start on a physical display.
- File and directory browsing with create-folder, rename, delete, copy, and move actions. Recursive directory deletion requires explicit confirmation; copy/move reject existing destinations and symbolic links, and source operations are restricted to direct children of the current directory.
- File opening is delegated to the host desktop. Copy/move supports regular files and directory trees; cross-filesystem moves use copy-then-remove with rollback, while recursive copy refuses symbolic links and special files.
- XDG desktop-entry parsing with dedicated unit tests, hidden/NoDisplay filtering, canonical-path allowlisting, and launch via QProcess argument lists.
- Host-native tool inventory based only on executables actually found on PATH, with extra checks for common Kali security tools.
- Persistent light/dark preference, in-shell section switcher (Ctrl+Alt+Tab), native compositor Alt+Tab window switching, and same-user process inspection/confirmed SIGTERM.
- Start search combines installed applications with direct shortcuts to Win-To-Linux sections; its Settings-result navigation is covered by the headless UI smoke test.
- Start/search Terminal launches an installed terminal emulator from PATH in the user's home directory; it reports when none is installed and never installs packages automatically.
- Session launchers sanitize inherited Qt platform/plugin overrides, covered by fake-process regression tests.
- Responsive Start, notification, and Quick Settings popups; Start menu contents scroll at short window heights, with minimum-size behavior asserted by the QML smoke test.
- Settings can launch the isolated nested Weston/Cage preview from the GUI after read-only readiness checks; the child process is tracked and the host login manager is untouched.
- File Manager has Home, Ctrl+L path selection, and F5 refresh; the headless smoke test exercises these actions against a temporary directory.
- Version and diagnostics CLI (`--version`, `--system`, `--session-check`, `--diagnostic-report`, `--preview-check`, and `--standalone-check`).
- Asynchronous package operations with bounded output, timeout/cancellation, PolicyKit through pkexec, explicit install/remove confirmation, and a clear preflight message when pkexec is missing.
- Wine/Bottles detection, confirmed Windows-file launch, and a managed isolated Wine-prefix workflow: create prefixes asynchronously, launch .exe/.msi files inside a selected prefix, and remove only validated managed prefixes after confirmation.
- Wine/XWayland compatibility scope and per-app validation checklist ([guide](docs/wine-compatibility.md)); CI verifies Wine's built-in Notepad remains open under nested XWayland, but third-party Windows apps are not certified.
- A dedicated desktop-entry launch integration test exercises real `QProcess` launching against a fake executable and verifies that shell metacharacters are passed literally.
- Per-user install/uninstall scripts, desktop launcher, C++ unit tests, headless QML smoke test, install-staging checks, and GitHub Actions CI.
- CI launches the actual Qt shell in a disposable nested Weston Wayland session (software rendering) and uses a bounded gdb-backed startup smoke to detect crashes; taskbar icon controls include accessible names, and the headless functional test checks taskbar actions plus common keyboard shortcuts.
- Full-app build/test matrix for Ubuntu 24.04, Kali Rolling, Fedora, Rocky Linux 10, Arch Linux, and openSUSE Tumbleweed (`.github/workflows/full-build-matrix.yml`); all six jobs passed on [the latest verified six-distro run](https://github.com/pctaha501-create/Win-To-Linux/actions/runs/38890368511).

## Command-line diagnostics

- `win-to-linux --version` prints the build version without opening the graphical shell.
- `win-to-linux --system` and `win-to-linux --session-check` print host/session information.
- `win-to-linux --diagnostic-report` prints a support report; `--preview-check` and `--standalone-check` perform read-only readiness checks and return exit code 2 when blocked.

## Build and test on Ubuntu 24.04
Install build-essential, cmake, ninja-build, qt6-base-dev, qt6-wayland, qt6-declarative-dev, qt6-declarative-dev-tools, qml6-module-qtqml, qml6-module-qtqml-models, qml6-module-qtqml-workerscript, qml6-module-qtquick, qml6-module-qtquick-controls, qml6-module-qtquick-templates, qml6-module-qtquick-layouts, qml6-module-qtquick-window, and qml6-module-qtquick-dialogs. Then run: sh scripts/build.sh

## Install for the current user
    sh scripts/install-user.sh
    ~/.local/bin/Winux11
    ~/.local/bin/Winux11 --fullscreen
    ~/.local/bin/Winux11 --diagnostic-report
    ~/.local/bin/win-to-linux-isolated-preview

### Optional isolated preview

1. Install **Weston** (preferred for a nested multi-window desktop) or **Cage** (fallback) using your distribution's package manager; Win-To-Linux does not install either automatically.
2. Log in to an existing Wayland desktop session.
3. Run `~/.local/bin/win-to-linux-isolated-preview`.
4. Exit the preview to return to your existing desktop.

The launcher checks the current user's runtime directory and active Wayland socket. It deliberately refuses to run from a TTY or start a compositor on a physical display. When Weston is available the launcher uses its Wayland backend, a unique per-user socket, software rendering, and no user compositor config; otherwise it falls back to Cage. This is a nested preview only, not yet a standalone login session.

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

See docs/architecture.md, docs/installation.md, docs/roadmap.md, and docs/standalone-session.md for the explicit independent-session implementation and release gates.

## AI Screen Share (MCP)

Win-To-Linux includes an **AI Screen Share** section that creates a private session through the existing The End Screen Share service. Select **Create MCP**, open the generated host link on the computer to share, and approve the browser's native screen picker. The app also provides the existing remote MCP endpoint and a copyable connection guide for an AI client.

- MCP endpoint: `https://mcp.the-end.shop/mcp`
- Screen-share session API: `https://screen-share.the-end.shop/api/sessions`
- MCP tools: `create_screen_share_session`, `get_screen_share_status`, and `get_screen_snapshot`
- Only browser-approved screen capture is shared; the desktop app does not silently start capture or send keyboard/mouse input.
- Sessions expire after 60 minutes. **End session** revokes the tokens and stored snapshot; also stop sharing in the host browser when finished.
- The viewer token is a credential. Copy it only into an AI client you trust.

See [AI Screen Share setup and security](docs/ai-screen-share.md). The MCP endpoint requires its normal OAuth sign-in; creating a desktop session does not automatically configure a third-party AI client.
