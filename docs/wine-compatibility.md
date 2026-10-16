# Windows application compatibility (Wine / Bottles)

## Scope and guarantees

Win-To-Linux does not emulate Windows and cannot guarantee that a given Windows application will work. Wine translates Windows APIs; compatibility varies with the program, CPU architecture, graphics stack, fonts, runtime libraries, drivers, DRM/anti-cheat, and installer behavior. Bottles can simplify per-application prefixes but is not a substitute for Wine itself.

The shell only offers `.exe` and `.msi` launch when a Wine executable is found on `PATH`. It asks for confirmation before launch and passes arguments directly without invoking a shell. It does not install Wine or Bottles, elevate the selected Windows program, or bypass application licensing/security controls.

## Distribution packages

Use the package manager for the active base distribution; package names and 32-bit support vary by release. Examples (check the distribution's current docs first):

- Ubuntu/Debian: `wine` or the release's `wine64` package; optional 32-bit support may require enabling the architecture and installing matching packages.
- Fedora/RHEL family: Wine packages are commonly provided as `wine` / `wine-core`; availability varies by release and repositories.
- Arch Linux: `wine` (and optionally `wine-mono`, `wine-gecko`); multilib may be needed for 32-bit programs.
- openSUSE: Wine packages are available under names such as `wine` / `wine-32bit`, depending on release and repositories.

Do not copy commands for one distribution into another. Win-To-Linux reports the detected distribution but deliberately does not install these packages automatically.

## Safe troubleshooting order

1. Start with a program from a trusted source. An `.exe` or `.msi` can modify files in your Wine prefix and may access user files; confirmation is not a malware scanner.
2. Try a fresh, disposable Wine prefix for an unknown application rather than a prefix containing important application data. Back up any prefix before changing dependencies.
3. Confirm `wine --version` works in the same user account and that the program's architecture is supported.
4. If the application opens and immediately closes, launch it from a terminal in the same session and inspect the error output. Do not run the whole desktop as root.
5. For GUI applications in Win-To-Linux's experimental Weston session, the application must inherit the nested session's `DISPLAY` and, if Weston supplies one, its valid current-user Xauthority file. Never manually copy the host desktop's `DISPLAY` or `XAUTHORITY` into the nested session.
6. Test keyboard input, rendering, resizing, clipboard, audio, file access, and clean shutdown individually. A successful process start alone does not prove the application is usable.

## Current XWayland/session status

CI starts a disposable headless Weston parent and nested Weston, connects a real Wayland client, captures the nested XWayland environment, and connects `xdpyinfo` without host credentials. A separate Ubuntu 24.04 workflow initializes a disposable Wine prefix and verifies that Wine's built-in Notepad stays open under nested XWayland ([run](https://github.com/pctaha501-create/Win-To-Linux/actions/runs/38886382702)). This validates one basic real Wine GUI path only; it does not certify third-party Windows applications, installation workflows, audio, clipboard, or a physical VM session. The experimental standalone launcher uses a private session socket and a child-only compositor watchdog; it remains unvalidated on physical VM hardware.

## App-specific validation record

Before calling a Windows application supported, record:

- Application name/version and installer/source
- Distribution, kernel, graphics adapter/driver, and session backend
- Wine/Bottles version and prefix architecture
- Install result and launch logs
- Rendering, keyboard/mouse, resize/focus, clipboard, audio, file access, and shutdown results
- Known issues and exact reproduction steps

No app-specific Wine GUI compatibility claim is made until this record has been completed on a supported VM/session.


## Isolated Wine prefixes in Win-To-Linux

The Windows Apps page can create isolated prefixes under `~/.local/share/win-to-linux/wine-prefixes`. A prefix is created with `wineboot -u` in an asynchronous child process and appears in the selector only after Wine has initialized its `system.reg` registry file. Launching an `.exe` or `.msi` in a selected prefix sets `WINEPREFIX` for that child process only; it does not change the environment of the Win-To-Linux shell or other applications. Every launch requires the existing confirmation dialog.

Prefix names are limited to 1–40 ASCII letters, digits, dots, underscores and hyphens, must begin with a letter or digit, and cannot contain path separators. The manager rejects symlinked prefix directories and validates that operations remain within its managed root. Removing a prefix requires an explicit confirmation and permanently removes the applications, registry and data stored inside that prefix. Back up important prefix data first. This feature does not install Wine, download Windows installers, or guarantee third-party application compatibility. `wineboot` and `wine` must both be installed and available on `PATH`.
