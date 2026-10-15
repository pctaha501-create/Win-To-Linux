# Roadmap and completion criteria

"100%" applies to a defined milestone, not universal compatibility with every Linux distribution and Windows application.

## Preview shell structure
- [x] C++20/CMake/Ninja foundation
- [x] Distribution and package-manager detection
- [x] Safe distro-specific package command plans
- [x] Qt Quick navigation and dashboard
- [x] Host system details
- [x] XDG app discovery, search and launch
- [x] Native-tool discovery based on executables actually present on PATH
- [x] Persistent appearance setting
- [x] procfs process list and confirmation-gated same-user SIGTERM
- [x] Wine/Bottles detection and confirmation-gated .exe/.msi launch via Wine
- [x] Per-user install/uninstall scripts and desktop entry
- [x] Unit tests and headless QML smoke test defined
- [x] Shell-script syntax and CI install-staging checks defined
- [ ] CI build, unit tests, QML smoke test, and install staging all pass on latest commit
- [ ] Interactive VM check for keyboard/focus/accessibility and responsive layout

## Next milestone
- [ ] Async package operation controller with timeout, cancellation, stdout/stderr
- [ ] Explicit package-operation confirmation and PolicyKit integration
- [ ] CI matrix across Ubuntu, Fedora, Arch, and openSUSE
- [ ] Compositor/session launcher with recovery and isolated Wayland readiness checks
- [ ] Process tree, notifications, richer settings
- [ ] Wine/Bottles compatibility notes and app-specific testing

## Release gate
A milestone is ready for user testing only after CI passes, automated tests pass, the UI starts on a supported desktop, and limitations are documented. Do not switch the login manager as part of preview-shell testing.
