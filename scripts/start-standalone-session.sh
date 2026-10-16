#!/bin/sh
set -eu
fail() { printf 'Win-To-Linux standalone session: %s\n' "$*" >&2; exit 2; }
[ "$(uname -s)" = Linux ] || fail "this launcher is Linux-only."
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
[ "${WTL_ENABLE_DRM_SESSION:-}" = 1 ] || fail "experimental DRM startup is disabled; set WTL_ENABLE_DRM_SESSION=1 only from a Linux virtual terminal after reading docs/standalone-session.md."
[ -z "${WAYLAND_DISPLAY:-}" ] || fail "an existing Wayland session is active; use the isolated preview launcher instead."
[ -z "${DISPLAY:-}" ] || fail "an X11 display is active; refusing to take over the host desktop. Switch to a text virtual terminal first."
tty_path=$(tty 2>/dev/null || true)
case "$tty_path" in
    /dev/tty[0-9]*) tty_number=${tty_path#/dev/tty}; case "$tty_number" in ''|*[!0-9]*) fail "could not determine the active Linux virtual terminal." ;; esac ;;
    *) fail "run this experimental launcher from a real Linux virtual terminal (for example Ctrl+Alt+F3), not from a terminal emulator." ;;
esac
runtime_dir=${XDG_RUNTIME_DIR:-}
[ -n "$runtime_dir" ] || fail "XDG_RUNTIME_DIR is not set; log in on a local text virtual terminal first."
[ -d "$runtime_dir" ] && [ -w "$runtime_dir" ] || fail "XDG_RUNTIME_DIR is missing or not writable."
runtime_owner=$(stat -c '%u' -- "$runtime_dir" 2>/dev/null || true)
runtime_mode=$(stat -c '%a' -- "$runtime_dir" 2>/dev/null || true)
[ "$runtime_owner" = "$(id -u)" ] || fail "XDG_RUNTIME_DIR is not owned by the current user."
case "$runtime_mode" in *[!0-7]*|'') fail "could not verify XDG_RUNTIME_DIR permissions." ;; esac
[ $((0$runtime_mode & 077)) -eq 0 ] || fail "XDG_RUNTIME_DIR is accessible to other users; use a private mode-0700 runtime directory."
weston_bin=$(command -v weston 2>/dev/null || true)
[ -n "$weston_bin" ] || fail "Weston is not installed; install it using your distribution's package manager."
app_bin=${WTL_BIN:-}
if [ -z "$app_bin" ]; then app_bin=$(command -v win-to-linux 2>/dev/null || true); fi
if [ -z "$app_bin" ]; then
    candidate=$script_dir/win-to-linux
    [ -x "$candidate" ] && app_bin=$candidate
    if [ -z "$app_bin" ]; then
        repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
        candidate=$repo_root/build/src/app/win-to-linux
        [ -x "$candidate" ] && app_bin=$candidate
    fi
fi
[ -n "$app_bin" ] && [ -x "$app_bin" ] || fail "win-to-linux was not found; build or install the application first."
xwayland_capture=$script_dir/capture-xwayland-env.sh
[ -f "$xwayland_capture" ] || fail "the XWayland environment helper is missing; reinstall Win-To-Linux."
socket_name=win-to-linux-session-$(id -u)-$$
socket_path=$runtime_dir/$socket_name
[ ! -e "$socket_path" ] || fail "the session socket already exists: $socket_path"
weston_log=$(mktemp "$runtime_dir/win-to-linux-session.log.XXXXXX") || fail "could not create a private Weston log."
weston_config=$(mktemp "$runtime_dir/win-to-linux-session.ini.XXXXXX") || { rm -f -- "$weston_log"; fail "could not create a private Weston config."; }
xwayland_env_file=$(mktemp "$runtime_dir/win-to-linux-xwayland-env.XXXXXX") || { rm -f -- "$weston_log" "$weston_config"; fail "could not create a private XWayland environment file."; }
xwayland_wrapper=$(mktemp "$runtime_dir/win-to-linux-xwayland-capture.XXXXXX") || { rm -f -- "$weston_log" "$weston_config" "$xwayland_env_file"; fail "could not create a private XWayland helper wrapper."; }
cat > "$xwayland_wrapper" <<'EOF'
#!/bin/sh
exec sh "$WTL_XWAYLAND_HELPER"
EOF
chmod 700 "$xwayland_wrapper"
cat > "$weston_config" <<EOF
[core]
shell=desktop
xwayland=true
[shell]
panel-position=top
locking=false
animation=none
close-animation=none
startup-animation=none
focus-animation=none
[autolaunch]
path=$xwayland_wrapper
watch=false
EOF
chmod 600 "$weston_config"
weston_pid=
app_pid=
watchdog_pid=
watchdog_marker="$runtime_dir/win-to-linux-watchdog-$$.failed"
cleanup() {
    if [ -n "$watchdog_pid" ]; then kill "$watchdog_pid" >/dev/null 2>&1 || true; wait "$watchdog_pid" >/dev/null 2>&1 || true; fi
    if [ -n "$app_pid" ]; then kill "$app_pid" >/dev/null 2>&1 || true; wait "$app_pid" >/dev/null 2>&1 || true; fi
    if [ -n "$weston_pid" ]; then kill "$weston_pid" >/dev/null 2>&1 || true; wait "$weston_pid" >/dev/null 2>&1 || true; fi
    rm -f -- "$weston_log" "$weston_config" "$xwayland_env_file" "$xwayland_env_file.tmp."* "$xwayland_wrapper" "$socket_path" "$watchdog_marker"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM HUP
child_running() {
    child_state=$(ps -o stat= -p "$1" 2>/dev/null | tr -d ' ')
    [ -n "$child_state" ] || return 1
    case "$child_state" in Z*|X*) return 1 ;; *) return 0 ;; esac
}
printf 'Starting experimental Win-To-Linux on virtual terminal %s via Weston DRM. The login manager is not modified.\n' "$tty_number"
WTL_XWAYLAND_ENV_FILE="$xwayland_env_file" WTL_XWAYLAND_HELPER="$xwayland_capture" "$weston_bin" --backend=drm --tty="$tty_number" --socket="$socket_name" --renderer=pixman --xwayland --config="$weston_config" >"$weston_log" 2>&1 &
weston_pid=$!
i=0
while [ ! -S "$socket_path" ] && [ "$i" -lt 100 ]; do
    child_running "$weston_pid" || break
    sleep 0.1
    i=$((i+1))
done
if [ ! -S "$socket_path" ]; then cat "$weston_log" >&2; fail "Weston DRM did not create its Wayland socket; the host session was not changed."; fi
xwayland_display=
xwayland_authority=
i=0
while [ ! -s "$xwayland_env_file" ] && [ "$i" -lt 50 ]; do
    child_running "$weston_pid" || break
    sleep 0.1
    i=$((i+1))
done
if [ -s "$xwayland_env_file" ]; then
    xwayland_display=$(sed -n '1p' "$xwayland_env_file")
    xwayland_authority=$(sed -n '2p' "$xwayland_env_file")
    if ! printf '%s' "$xwayland_display" | grep -Eq '^:[0-9]+(\.[0-9]+)?$'; then xwayland_display=; xwayland_authority=; fi
    if [ -n "$xwayland_authority" ]; then [ -f "$xwayland_authority" ] && [ "$(stat -c '%u' -- "$xwayland_authority" 2>/dev/null || true)" = "$(id -u)" ] || xwayland_authority=; fi
fi
launch_shell() {
    # Only supervise the compositor child this launcher created; never signal the host session.
    "$@" &
    app_pid=$!
    (
        while child_running "$weston_pid"; do sleep 0.2; done
        if child_running "$app_pid"; then
            printf '%s\n' 'Weston exited while Win-To-Linux was running.' > "$watchdog_marker"
            kill -TERM "$app_pid" >/dev/null 2>&1 || true
        fi
    ) &
    watchdog_pid=$!
    if wait "$app_pid"; then app_status=0; else app_status=$?; fi
    app_pid=
    kill "$watchdog_pid" >/dev/null 2>&1 || true
    wait "$watchdog_pid" >/dev/null 2>&1 || true
    watchdog_pid=
    if [ -s "$watchdog_marker" ]; then cat "$watchdog_marker" >&2; return 2; fi
    return "$app_status"
}
if [ -n "$xwayland_display" ] && [ -n "$xwayland_authority" ]; then
    launch_shell env -u QT_QPA_PLATFORM -u QT_QPA_PLATFORM_PLUGIN_PATH -u QT_PLUGIN_PATH XDG_SESSION_TYPE=wayland WAYLAND_DISPLAY="$socket_name" DISPLAY="$xwayland_display" XAUTHORITY="$xwayland_authority" "$app_bin" --fullscreen
elif [ -n "$xwayland_display" ]; then
    launch_shell env -u XAUTHORITY -u QT_QPA_PLATFORM -u QT_PLUGIN_PATH -u QT_QPA_PLATFORM_PLUGIN_PATH XDG_SESSION_TYPE=wayland WAYLAND_DISPLAY="$socket_name" DISPLAY="$xwayland_display" "$app_bin" --fullscreen
else
    printf 'Warning: XWayland environment was not captured; X11-only apps may not launch.\n' >&2
    launch_shell env -u DISPLAY -u XAUTHORITY -u QT_QPA_PLATFORM -u QT_QPA_PLATFORM_PLUGIN_PATH -u QT_PLUGIN_PATH XDG_SESSION_TYPE=wayland WAYLAND_DISPLAY="$socket_name" "$app_bin" --fullscreen
fi
