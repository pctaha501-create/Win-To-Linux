# Roadmap and completion criteria

"100%" applies to a defined milestone, not universal compatibility with every Linux distribution and Windows application.

## Preview shell structure
- [x] C++20/CMake/Ninja foundation
- [x] Distribution and package-manager detection
- [x] Safe distro-specific package command plans
- [x] Qt Quick navigation and dashboard
- [x] Host system details and headless system diagnostics
- [x] XDG app discovery, search and launch
- [x] Native-tool discovery based on executables actually present on PATH
- [x] Read-only file and directory browser
- [x] Persistent appearance setting
- [x] procfs process list and confirmation-gated same-user SIGTERM
- [x] Wine/Bottles detection and confirmation-gated .exe/.msi launch via Wine
- [x] Asynchronous package operations with confirmation, timeout, cancellation, output and exit status
- [x] Per-user install/uninstall scripts and desktop entry
- [x] Unit tests for distro/package planning, file listing/navigation, and process metadata
- [x] Headless QML smoke test, shell-script syntax and install-staging checks defined
- [ ] CI build, all unit tests, QML smoke test, install staging, and headless diagnostics pass on latest commit
- [ ] Interactive VM check for keyboard/focus/accessibility and responsive layout

## Next milestone
- [ ] CI matrix across Ubuntu, Fedora, Arch, and openSUSE
- [ ] Compositor/session launcher with recovery and isolated Wayland readiness checks
- [ ] Process tree, notifications, richer settings
- [ ] Wine/Bottles compatibility notes and app-specific testing
- [ ] Robust XDG Desktop Entry parsing and launch integration tests

## Release gate
A milestone is ready for user testing only after CI passes, automated tests pass, the UI starts on a supported desktop, and limitations are documented. Do not switch the login manager as part of preview-shell testing.
