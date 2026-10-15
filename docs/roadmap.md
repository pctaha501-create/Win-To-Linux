# Roadmap and completion criteria

"100%" applies to a defined milestone, not universal compatibility with every Linux distribution and Windows application.

## Foundation
- [x] C++20/CMake/Ninja project
- [x] Distribution metadata and package-manager discovery
- [x] Safe distro-specific package command planning
- [x] Unit tests and CI workflow

## Preview shell
- [x] Qt Quick navigation and dashboard
- [x] Host distribution, kernel, architecture and hostname display
- [x] XDG desktop-entry discovery and search
- [x] Launch desktop entries without directly invoking a shell
- [x] Session-only dark/light switch
- [x] Headless QML smoke-test target in CTest
- [ ] CI result confirms build, unit test, and QML smoke test
- [ ] Interactive VM check for keyboard/focus/accessibility and responsive layout

## Operational desktop
- [ ] Persist preferences
- [ ] Process monitoring and safe termination flow
- [ ] Session isolation, compositor strategy, and recovery
- [ ] Notifications and richer settings

## Package operations
- [ ] Async process controller with timeout, cancellation, stdout/stderr
- [ ] Explicit confirmation and PolicyKit integration
- [ ] Integration tests for package manager exit codes
- [ ] CI matrix across Ubuntu, Fedora, Arch, and openSUSE

## Windows app compatibility
- [ ] Detect Wine/Bottles/Proton without pretending they exist
- [ ] Safe EXE/MSI launch flow and per-app caveats
- [ ] Never silently install runtimes or alter the host

## Release gate
A milestone is ready for user testing only after CI passes, automated tests pass, the UI starts on a supported desktop, and limitations are documented. Do not switch the login manager as part of preview-shell testing.
