# Architecture

1. Linux host: distribution, kernel, session, devices, native apps.
2. Core: host discovery, capability detection, safe command plans and session contracts.
3. Desktop shell (next milestone): Qt Quick shell, taskbar, launcher, navigation, notifications and settings.
4. Services: process/app discovery and explicitly user-triggered package operations.
5. Compatibility (later): Wine/Bottles integration and per-app compatibility reporting.

Win-To-Linux is not a new kernel. It must not hide the host distribution's native tools.

The detector reads key/value lines from /etc/os-release; it never sources the file or runs its contents. Package-manager discovery checks executable files on PATH and stores the absolute path. Unknown systems remain generic.

The command planner creates a program and argument list but does not execute it. Package names are validated and never interpolated into shell strings. Install/remove plans use pkexec for authentication. The future GUI must require explicit confirmation and report cancellation, timeout, and process errors.

Current tests validate parsing and command plans only; they do not claim that real package transactions or graphical sessions work on every distribution. Do not replace or disable the host display manager until a new graphical session has been validated in a recoverable test environment.
