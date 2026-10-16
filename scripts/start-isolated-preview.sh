#!/bin/sh
set -eu

fail() {
    printf 'Win-To-Linux isolated preview: %s\n' "$*" >&2
    exit 2
}

[ "$(uname -s)" = "Linux" ] || fail "this launcher is Linux-only."
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

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
    weston_log=$(mktemp "$runtime_dir/win-to-linux-weston.log.XXXXXX") || fail "could not create a private Weston log file."
    weston_config=$(mktemp "$runtime_dir/win-to-linux-weston.ini.XXXXXX") || { rm -f -- "$weston_log"; fail "could not create a private Weston config."; }
    xwayland_env_file=$(mktemp "$runtime_dir/win-to-linux-xwayland-env.XXXXXX") || { rm -f -- "$weston_log" "$weston_config"; fail "could not create a private XWayland environment file."; }
    xwayland_capture=$script_dir/capture-xwayland-env.sh
    xwayland_wrapper=$(mktemp "$runtime_dir/win-to-linux-xwayland-capture.XXXXXX") || { rm -f -- "$weston_log" "$weston_config" "$xwayland_env_file"; fail "could not create a private XWayland helper wrapper."; }
    [ -f "$xwayland_capture" ] || { rm -f -- "$weston_log" "$weston_config" "$xwayland_env_file" "$xwayland_wrapper"; fail "the XWayland environment helper is missing; reinstall Win-To-Linux."; }
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
panel-position=none
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
    cleanup_weston() {
        if [ -n "$weston_pid" ]; then kill "$weston_pid" >/dev/null 2>&1 || true; wait "$weston_pid" >/dev/null 2>&1 || true; fi
        rm -f -- "$weston_log" "$weston_config" "$xwayland_env_file" "$xwayland_env_file.tmp."* "$xwayland_wrapper" "$socket_path"
    }
    trap cleanup_weston EXIT HUP INT TERM
    printf 'Starting a nested Weston desktop with software rendering, a private config and a unique user socket. The host desktop and login manager will not be changed.\n'
    WTL_XWAYLAND_ENV_FILE="$xwayland_env_file" WTL_XWAYLAND_HELPER="$xwayland_capture" "$weston_bin" --backend=wayland --display="$wayland_display" --socket="$socket_name" --renderer=pixman --xwayland --config="$weston_config" >"$weston_log" 2>&1 &
    weston_pid=$!
    i=0
    while [ ! -S "$socket_path" ] && [ "$i" -lt 100 ]; do
        kill -0 "$weston_pid" 2>/dev/null || break
        sleep 0.1
        i=$((i+1))
    done
    if [ ! -S "$socket_path" ]; then
        cat "$weston_log" >&2
        if [ -n "$cage_bin" ]; then
            printf 'Nested Weston failed to initialize; falling back to Cage without changing the host session.\n' >&2
            cleanup_weston
            weston_pid=
            trap - EXIT HUP INT TERM
            exec env -u DISPLAY -u XAUTHORITY -u QT_QPA_PLATFORM -u QT_QPA_PLATFORM_PLUGIN_PATH -u QT_PLUGIN_PATH XDG_SESSION_TYPE=wayland "$cage_bin" -- "$app_bin" --fullscreen
        fi
        fail "nested Weston did not create its Wayland socket and Cage is unavailable."
    fi
    xwayland_display=
    xwayland_authority=
    i=0
    while [ ! -s "$xwayland_env_file" ] && [ "$i" -lt 50 ]; do
        kill -0 "$weston_pid" 2>/dev/null || break
        sleep 0.1
        i=$((i+1))
    done
    if [ -s "$xwayland_env_file" ]; then
        xwayland_display=$(sed -n '1p' "$xwayland_env_file")
        xwayland_authority=$(sed -n '2p' "$xwayland_env_file")
        if ! printf '%s' "$xwayland_display" | grep -Eq '^:[0-9]+(\.[0-9]+)?$'; then xwayland_display=; xwayland_authority=; fi
        if [ -n "$xwayland_authority" ]; then
            [ -f "$xwayland_authority" ] && [ "$(stat -c '%u' -- "$xwayland_authority" 2>/dev/null || true)" = "$(id -u)" ] || xwayland_authority=
        fi
    fi
    if [ -n "$xwayland_display" ]; then
        printf 'Nested XWayland display: %s\n' "$xwayland_display"
        if [ -n "$xwayland_authority" ]; then
            set +e
            env -u QT_QPA_PLATFORM -u QT_QPA_PLATFORM_PLUGIN_PATH -u QT_PLUGIN_PATH XDG_SESSION_TYPE=wayland WAYLAND_DISPLAY="$socket_name" DISPLAY="$xwayland_display" XAUTHORITY="$xwayland_authority" "$app_bin" --maximized
            app_status=$?
            set -e
        else
            printf 'Nested XWayland has no exported authority path; exposing only its DISPLAY and explicitly clearing host XAUTHORITY.\n' >&2
            set +e
            env -u XAUTHORITY -u QT_QPA_PLATFORM -u QT_QPA_PLATFORM_PLUGIN_PATH -u QT_PLUGIN_PATH XDG_SESSION_TYPE=wayland WAYLAND_DISPLAY="$socket_name" DISPLAY="$xwayland_display" "$app_bin" --maximized
            app_status=$?
            set -e
        fi
    else
        printf 'Warning: nested XWayland environment was not captured; X11-only applications may not launch.\n' >&2
        set +e
        env -u DISPLAY -u XAUTHORITY -u QT_QPA_PLATFORM -u QT_QPA_PLATFORM_PLUGIN_PATH -u QT_PLUGIN_PATH XDG_SESSION_TYPE=wayland WAYLAND_DISPLAY="$socket_name" "$app_bin" --maximized
        app_status=$?
        set -e
    fi
    set -e
    cleanup_weston
    weston_pid=
    trap - EXIT HUP INT TERM
    exit "$app_status"
fi
printf 'Starting the Win-To-Linux preview inside nested Cage. Your existing login manager and desktop session will not be changed.\n'
exec env -u DISPLAY -u XAUTHORITY -u QT_QPA_PLATFORM -u QT_QPA_PLATFORM_PLUGIN_PATH -u QT_PLUGIN_PATH XDG_SESSION_TYPE=wayland "$cage_bin" -- "$app_bin" --fullscreen
