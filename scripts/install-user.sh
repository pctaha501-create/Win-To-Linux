#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
PREFIX="${PREFIX:-$HOME/.local}"
CONFIG_HOME="${XDG_CONFIG_HOME:-$HOME/.config}"
UNIT_DIR="$CONFIG_HOME/systemd/user"
UNIT_SOURCE="$ROOT/packaging/systemd/user/Win-To-Linux.service"

[ "$(uname -s)" = Linux ] || { echo "Win-To-Linux is Linux-only." >&2; exit 1; }
[ -f "$UNIT_SOURCE" ] || { echo "Missing user service: $UNIT_SOURCE" >&2; exit 1; }
echo "Building and testing Win-To-Linux before installation."
sh "$ROOT/scripts/build.sh"
cmake --install "$BUILD_DIR" --prefix "$PREFIX"
mkdir -p "$UNIT_DIR"
install -m 0644 "$UNIT_SOURCE" "$UNIT_DIR/Win-To-Linux.service"
if command -v systemctl >/dev/null 2>&1 && [ -n "${XDG_RUNTIME_DIR:-}" ]; then
    systemctl --user daemon-reload || echo "Warning: user systemd is unavailable; run systemctl --user daemon-reload after login." >&2
fi
printf '\nInstalled Win-To-Linux for user %s in %s.\n' "$(id -un)" "$PREFIX"
printf 'In an existing Wayland desktop, run:\n  systemctl --user daemon-reload\n  systemctl --user start Win-To-Linux.service\n'
printf 'Launch the shell directly with: %s/bin/Winux11\n' "$PREFIX"
printf 'Show diagnostics with: %s/bin/Winux11 --diagnostic-report\n' "$PREFIX"
printf 'Restart later with: systemctl --user restart Win-To-Linux.service\n'
printf 'This is a nested preview; it does not replace your desktop or display manager.\n'
