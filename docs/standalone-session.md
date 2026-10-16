# Standalone session implementation gate

## Current state

Win-To-Linux is a Qt Quick shell running inside an existing desktop. `start-isolated-preview.sh` uses Cage only as a nested preview and deliberately requires an existing user-owned Wayland socket. This is not an independent compositor or login session.

## Target architecture

1. **Session backend interface** — define a small C++ interface for capability probing, start/stop, socket discovery, health checks, and diagnostic output. Backend selection must be explicit and testable; the shell UI must not start privileged display services.
2. **Wayland compositor backend** — use a maintained compositor framework (evaluate wlroots versus Qt Wayland Compositor against distro availability, licensing, Qt integration, and testability) rather than attempting direct DRM/EGL from the existing GUI process. Keep compositor code in a separate executable/target.
3. **Session supervisor** — start the compositor as the current user, use a unique socket under a verified `$XDG_RUNTIME_DIR`, wait with a bounded timeout for socket creation and health, then launch the shell with only the new session environment. Capture logs and propagate exit status.
4. **Recovery** — stop only child processes started by the supervisor; clean up only the socket/runtime files it created; use bounded retries and backoff. Never terminate the host compositor, alter GDM/SDDM, or write into `/usr/share/wayland-sessions` as part of the preview installer.
5. **Window management** — implement Wayland xdg-shell roles, configure/acknowledge handshakes, focus, keyboard/pointer routing, resize/move, close, and taskbar state before advertising the session as a desktop. The current QML page switcher is navigation inside one application window, not a compositor window manager.
6. **Distribution adapters** — detect compositor build/runtime dependencies and packaging capability separately for Debian/Ubuntu, Fedora/RHEL, Arch, and openSUSE. Never assume apt, auto-install packages, or change the base distribution.

## Release gates

- Unit-test backend selection, environment construction, path ownership, timeout handling, cleanup boundaries, and restart limits.
- CI builds the shell and supervisor independently; missing optional compositor dependencies must produce a clear configure-time message rather than breaking the preview build.
- Integration test starts a nested compositor only in a disposable headless/virtualized environment and verifies a real Wayland client connection.
- VM test validates keyboard, pointer, resize, close, focus, multi-window stacking, clipboard, DPI, suspend/resume, and recovery after a compositor crash.
- A session entry is not shipped until the supported compositor backend has passed the VM gate.

## Safety constraints

Do not run a new compositor directly against physical DRM/KMS in CI or on the user's current VM. Do not disable the host display manager, change the default session, or modify system package state automatically. If the compositor backend is missing or fails health checks, exit cleanly and leave the existing desktop intact.
