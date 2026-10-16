#!/bin/sh
set -eu

fail() {
    printf 'Win-To-Linux isolated preview: %s\n' "$*" >&2
    exit 2
}

[ "$(uname -s)" = "Linux" ] || fail "this launcher is Linux-only."

runtime_dir=${XDG_RUNTIME_DIR:-}
wayland_display=${WAYLAND_DISPLAY:-}
[ -n "$runtime_dir" ] || fail "XDG_RUNTIME_DIR is not set. Start this from an existing Wayland desktop."
[ -d "$runtime_dir" ] && [ -w "$runtime_dir" ] || fail "XDG_RUNTIME_DIR is missing or not writable."
runtime_owner=$(stat -c '%u' -- "$runtime_dir" 2>/dev/null || true)
[ "$runtime_owner" = "$(id -u)" ] || fail "XDG_RUNTIME_DIR is not owned by the current user."
[ -n "$wayland_display" ] || fail "WAYLAND_DISPLAY is not set. Refusing to start a compositor on a physical display."

case "$wayland_display" in
    /*) wayland_socket=$wayland_display ;;
    *) wayland_socket=$runtime_dir/$wayland_display ;;
esac
[ -S "$wayland_socket" ] || fail "the active Wayland socket was not found at $wayland_socket."

cage_bin=$(command -v cage 2>/dev/null || true)
[ -n "$cage_bin" ] || fail "Cage is not installed. Install it with your distribution's package manager; this script will not install packages automatically."

app_bin=${WTL_BIN:-}
if [ -z "$app_bin" ]; then
    app_bin=$(command -v win-to-linux 2>/dev/null || true)
fi
if [ -z "$app_bin" ]; then
    script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
    sibling=$script_dir/win-to-linux
    [ -x "$sibling" ] && app_bin=$sibling
    if [ -z "$app_bin" ]; then
        repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
        candidate=$repo_root/build/src/app/win-to-linux
        [ -x "$candidate" ] && app_bin=$candidate
    fi
fi
[ -n "$app_bin" ] && [ -x "$app_bin" ] || fail "win-to-linux was not found. Install it for your user or build the project first."

if ! readiness=$("$app_bin" --preview-check 2>&1); then
    printf '%s\n' "$readiness" >&2
    fail "readiness preflight refused to launch the nested preview."
fi

printf 'Starting the Win-To-Linux preview inside nested Cage. Your existing login manager and desktop session will not be changed.\n'
exec "$cage_bin" -- "$app_bin" --fullscreen
