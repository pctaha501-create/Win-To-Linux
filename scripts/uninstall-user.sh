#!/usr/bin/env sh
set -eu
PREFIX="${PREFIX:-$HOME/.local}"
CONFIG_HOME="${XDG_CONFIG_HOME:-$HOME/.config}"
UNIT_DIR="$CONFIG_HOME/systemd/user"
if command -v systemctl >/dev/null 2>&1 && [ -n "${XDG_RUNTIME_DIR:-}" ]; then
    systemctl --user disable --now Win-To-Linux.service >/dev/null 2>&1 || true
fi
rm -f -- "$UNIT_DIR/Win-To-Linux.service"
if command -v systemctl >/dev/null 2>&1 && [ -n "${XDG_RUNTIME_DIR:-}" ]; then
    systemctl --user daemon-reload >/dev/null 2>&1 || true
fi
rm -f -- "$PREFIX/bin/win-to-linux" "$PREFIX/bin/win-to-linux-isolated-preview" "$PREFIX/bin/win-to-linux-standalone-session" "$PREFIX/bin/capture-xwayland-env.sh" "$PREFIX/share/applications/win-to-linux.desktop"
echo "Removed Win-To-Linux executables, desktop launcher, and user systemd service from $PREFIX."
echo "Source, build output, and user settings were left untouched."
