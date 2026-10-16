# Roadmap and completion criteria

"100%" applies to a defined milestone, not universal compatibility with every Linux distribution and Windows application.

## Preview shell structure
- [x] C++20/CMake/Ninja foundation
- [x] Distribution and package-manager detection
- [x] Safe distro-specific package command plans
- [x] Qt Quick navigation and dashboard
- [x] Host system details and headless system diagnostics
- [x] XDG app discovery/search and launch using a tested desktop-entry parser and canonical-path allowlisting
- [x] Native-tool discovery based on executables actually present on PATH
- [x] File and directory browser with create, rename, confirmed delete, copy and move operations; copy/move reject overwrites and symbolic links
- [x] Persistent appearance setting
- [x] Parent-first procfs process tree with PID/PPID, search/refresh controls, and confirmation-gated same-user SIGTERM
- [x] Task Manager CPU/RAM summaries, process sorting, and transient toast feedback for refresh/stop actions
- [x] Persistent notification center and keyboard shortcuts for common shell actions
- [x] Settings diagnostics export to clipboard, including runtime safety and detected compositor executable paths
- [x] Read-only `--preview-check` CLI reports all nested-preview blockers and exits 2 on an unsafe/headless environment; CI asserts the refusal path
- [x] Keyboard shortcut reference in Settings
- [x] Workspace switcher with Alt+Tab previous-workspace navigation and Ctrl+Alt+Tab overview; headless smoke test asserts its QML controls
- [x] Wine/Bottles detection and confirmation-gated .exe/.msi launch via Wine
- [x] Asynchronous package operations with confirmation, timeout, cancellation, output and exit status
- [x] Per-user install/uninstall scripts and desktop entry
- [x] Unit tests for distro/package planning, file listing/navigation, safe file operations and copy/move, process metadata, and package-name validation
- [x] Headless QML smoke test instantiates the file-manager and package-manager pages, with shell-script syntax and install-staging checks
- [x] CI build, all unit tests, QML smoke test, install/uninstall staging, and headless diagnostics pass on verified commit `5037582a72e99beab9d56372eb14cfbf1f5640c6`
- [ ] Interactive VM check for keyboard/focus/accessibility and responsive layout

## Next milestone
- [x] Distro-detector CI matrix across Ubuntu, Fedora, Arch, and openSUSE
- [x] Full application build/test matrix across Ubuntu 24.04, Fedora, Arch Linux, and openSUSE Tumbleweed; all four jobs passed on commit `312b0611976f3117628076e9e2971cb06ca4fbf8`
- [x] Guarded nested Wayland preview launcher with runtime ownership/socket checks (preview only; not a standalone session)
- [ ] Full compositor/session launcher with recovery and a supported independent desktop session (implementation gates: docs/standalone-session.md)
- [x] Persistent notifications, richer settings, copyable diagnostics, and nested-preview readiness blocker list
- [ ] Wine/Bottles compatibility notes and app-specific testing
- [ ] End-to-end desktop launch integration tests across supported desktop environments

## Release gate
A milestone is ready for user testing only after CI passes, automated tests pass, the UI starts on a supported desktop, and limitations are documented. Do not switch the login manager as part of preview-shell testing.
