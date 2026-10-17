# Roadmap and completion criteria

"100%" applies to a defined milestone, not universal compatibility with every Linux distribution and Windows application.

## Preview shell structure
- [x] C++20/CMake/Ninja foundation
- [x] Distribution and package-manager detection
- [x] Safe distro-specific package command plans
- [x] Qt Quick navigation and dashboard
- [x] Host system details and headless system diagnostics
- [x] XDG app discovery/search and launch using a tested desktop-entry parser, TryExec/working-directory handling, safe Exec field-code expansion, and canonical-path allowlisting
- [x] Native-tool discovery based on executables actually present on PATH
- [x] File and directory browser with create, rename, confirmed delete, copy and move operations; copy/move reject overwrites and symbolic links; cross-filesystem move uses copy-then-remove with rollback and has a conditional separate-filesystem regression test
- [x] Persistent appearance setting
- [x] Parent-first procfs process tree with PID/PPID, search/refresh controls, and confirmation-gated same-user SIGTERM
- [x] Task Manager CPU/RAM summaries, process sorting, and transient toast feedback for refresh/stop actions
- [x] Persistent notification center and keyboard shortcuts for common shell actions
- [x] Settings diagnostics export to clipboard, including runtime safety and detected compositor executable paths
- [x] Read-only `--preview-check` CLI reports all nested-preview blockers and exits 2 on an unsafe/headless environment; CI asserts the refusal path
- [x] Isolated-preview integration test uses a fake owned Wayland socket/Cage to verify preflight runs first and refuses unsafe launch (Linux CI passed on `8f83e02449d3e74e58ffc1069b81a2c8675dc005`)
- [x] Weston-first nested desktop backend: unique per-user socket, nested Wayland backend, pixman renderer, no user config, supervised cleanup, and Cage fallback; headless Weston plus real `wayland-info` client smoke test passed; private `panel-position=top` config that preserves native window/workspace controls and `--maximized` shell startup also passed CI on commit `e05a2f7762d0452d2401e0349be6039ba6ea35c2`
- [x] Keyboard shortcut reference in Settings
- [x] Start search offers installed-app results plus direct shell-section shortcuts; the headless functional smoke searches for Settings, verifies the result is visible, activates it, and checks navigation
- [x] Alt+Tab is left to the native compositor for real application-window switching; the shell smoke asserts no in-app Alt+Tab interceptor exists
- [x] Session launch regression tests inject host Qt platform/plugin overrides and verify the nested/standalone shell receives none of them
- [x] In-shell section switcher with Ctrl+Alt+Tab; Alt+Tab is left to the host/compositor for real application-window switching, and the smoke test asserts the shell does not intercept it
- [x] Wine/Bottles detection, confirmation-gated default-prefix launch, and isolated Wine-prefix management (asynchronous initialization, validated names/paths, per-launch `WINEPREFIX`, and confirmation-gated removal)
- [x] Asynchronous package operations with confirmation, timeout, cancellation, output and exit status; preflight reports missing `pkexec`/PolicyKit before starting
- [x] Per-user install/uninstall scripts and desktop entry
- [x] Installable `Winux11` command delegates to the same tested binary; desktop launcher and staged install/uninstall CI verify the alias
- [x] Unit tests for distro/package planning, file listing/navigation, safe file operations and copy/move, process metadata, and package-name validation
- [x] Desktop-entry launch integration test uses an isolated HOME and fake executable to verify TryExec, working directory, Exec field-code arguments, literal shell metacharacters, and refusal of missing executables/out-of-root entries
- [x] Start/search Terminal action launches the first installed terminal emulator found on PATH through `QProcess::startDetached` (no shell parsing); fake-executable integration test covers successful launch and the actionable missing-terminal path
- [x] Headless QML smoke test instantiates the file-manager and package-manager pages, with shell-script syntax and install-staging checks
- [x] CI starts the actual Qt shell as a Wayland client inside nested Weston with software rendering; a bounded gdb-backed startup smoke checks for crashes and QML/platform errors
- [x] Icon-only taskbar controls have explicit accessible names and stable object names; the application smoke test asserts the Start/search/system/packages/files/settings/notifications/quick-settings controls exist
- [x] Headless functional smoke dispatches Qt key events for Ctrl+Space, Ctrl+Shift+N, Ctrl+comma, Ctrl+E, Ctrl+Shift+Esc, and Ctrl+Alt+Tab; verifies popup visibility and page navigation, plus taskbar-button actions
- [x] Minimum-window QML smoke verifies Start/Notifications/Quick Settings fit within an 820×560 shell window and Start menu has a scrollable content extent; QML dialog implicit-width binding-loop warnings were removed
- [x] Settings launches the isolated nested preview only after readiness preflight, tracks the child process, and refuses to start in headless/no-Wayland contexts; no login manager or DRM is changed by this action
- [x] File Manager Home, Ctrl+L, and F5 actions are exercised against an isolated temporary directory by the headless QML integration smoke
- [x] Version CLI is sourced from the CMake project version and checked in CI; latest Linux CI passed on [run 38892769036](https://github.com/pctaha501-create/Win-To-Linux/actions/runs/38892769036)
- [x] CI build, all unit tests, QML smoke test, install/uninstall staging, and headless diagnostics pass on verified commit `5037582a72e99beab9d56372eb14cfbf1f5640c6`
- [x] Regression tests verify recent notifications persist across SystemInfo instances, are capped at eight, and diagnostic reports include runtime/compositor readiness and copy to clipboard
- [x] Application discovery and launch cover per-user Flatpak exports and custom XDG_DATA_DIRS roots; system Flatpak and Snap desktop-entry roots are included in the shared launch allowlist
- [ ] Interactive VM check for keyboard/focus/accessibility and responsive layout

## Next milestone
- [x] Distro-detector CI matrix across Ubuntu, Kali, Fedora/Rocky, Arch, and openSUSE
- [x] Full application build/test matrix across Ubuntu 24.04, Kali Rolling, Fedora, Rocky Linux 10, Arch Linux, and openSUSE Tumbleweed; all six jobs passed on [verified run 38892768912](https://github.com/pctaha501-create/Win-To-Linux/actions/runs/38892768912)
- [x] Guarded nested Wayland preview launcher with runtime ownership/socket checks (preview only; not a standalone session)
- [x] Read-only standalone readiness diagnostics in CLI and Settings; safely refuses non-interactive/headless environments
- [x] Experimental TTY-to-DRM Weston session launcher with strict opt-in, host-display refusal, private XWayland environment bridge, bounded socket wait, child-only cleanup, and a watchdog that terminates the shell if the owned compositor exits (fake-backend CI only; not yet VM-certified)
- [x] Fake-process regression tests cover standalone and nested Weston failure before socket creation and compositor crash after startup; supervisors escalate TERM to KILL after a bounded grace period, terminate only owned children, and clean private runtime artifacts
- [ ] Production compositor/login session, seat/input lifecycle, crash recovery, and supported independent desktop session (implementation gates: docs/standalone-session.md)
- [x] Introduce a testable session-backend interface and explicit backend registry plus shared, pure Weston-DRM and nested-preview probes used by CLI/Settings diagnostics; test refusal prerequisites and unknown-backend rejection without opening DRM
- [x] Persistent notifications, richer settings, copyable diagnostics, and nested-preview readiness blocker list
- [x] Wine/Bottles compatibility notes, distribution package guidance, and an app-specific validation template (`docs/wine-compatibility.md`)
- [x] Real Wine GUI smoke: Wine's built-in Notepad stayed open under nested Weston/XWayland with a disposable prefix in the dedicated Ubuntu 24.04 workflow ([run](https://github.com/pctaha501-create/Win-To-Linux/actions/runs/38886382702))
- [ ] App-specific testing for third-party Windows programs, plus physical VM validation of keyboard/input, rendering, focus, and session recovery
- [ ] End-to-end desktop launch integration tests across supported desktop environments

## Release gate
A milestone is ready for user testing only after CI passes, automated tests pass, the UI starts on a supported desktop, and limitations are documented. Do not switch the login manager as part of preview-shell testing.
