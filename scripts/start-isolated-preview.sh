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

weston_bin=$(command -v weston 2>/dev/null || true)
cage_bin=$(command -v cage 2>/dev/null || true)
[ -n "$weston_bin" ] || [ -n "$cage_bin" ] || fail "Neither Weston nor Cage is installed. Install one with your distribution's package manager; this script will not install packages automatically."

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

if [ -n "$weston_bin" ]; then
    socket_name=win-to-linux-$(id -u)-$$
    socket_path=$runtime_dir/$socket_name
    [ ! -e "$socket_path" ] || fail "the nested Weston socket path already exists: $socket_path"
    weston_log=$(mktemp "$runtime_dir/win-to-linux-weston.XXXXXX") || fail "could not create a private Weston log file."
    weston_pid=
    cleanup_weston() {
        if [ -n "$weston_pid" ]; then kill "$weston_pid" >/dev/null 2>&1 || true; wait "$weston_pid" >/dev/null 2>&1 || true; fi
        rm -f -- "$weston_log" "$socket_path"
    }
    trap cleanup_weston EXIT HUP INT TERM
    printf 'Starting a nested Weston desktop with software rendering and a unique user socket. The host desktop and login manager will not be changed.\n'
    "$weston_bin" --backend=wayland --display="$wayland_display" --socket="$socket_name" --renderer=pixman --no-config >"$weston_log" 2>&1 &
    weston_pid=$!
    i=0
    while [ ! -S "$socket_path" ] && [ "$i" -lt 100 ]; do
        kill -0 "$weston_pid" 2>/dev/null || break
        sleep 0.1
        i=$((i+1))
    done
    if [ ! -S "$socket_path" ]; then
        cat "$weston_log" >&2
        fail "nested Weston did not create its Wayland socket."
    fi
    set +e
    WAYLAND_DISPLAY="$socket_name" "$app_bin" --fullscreen
    app_status=$?
    set -e
    cleanup_weston
    weston_pid=
    trap - EXIT HUP INT TERM
    exit "$app_status"
fi
printf 'Starting the Win-To-Linux preview inside nested Cage. Your existing login manager and desktop session will not be changed.\n'
exec "$cage_bin" -- "$app_bin" --fullscreen
