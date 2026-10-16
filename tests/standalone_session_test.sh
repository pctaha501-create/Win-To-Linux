#!/usr/bin/env sh
set -eux
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TMP=$(mktemp -d)
cleanup() { rm -rf -- "$TMP"; }
trap cleanup EXIT HUP INT TERM
runtime=$TMP/runtime
fakebin=$TMP/bin
mkdir -m 700 "$runtime" "$fakebin"
cat > "$fakebin/tty" <<'EOF'
#!/bin/sh
printf '%s\n' /dev/tty9
EOF
cat > "$fakebin/weston" <<'EOF'
#!/bin/sh
printf '%s\n' "$@" > "$WTL_WESTON_LOG"
config=
socket_name=
for arg in "$@"; do case "$arg" in --config=*) config=${arg#--config=} ;; --socket=*) socket_name=${arg#--socket=} ;; esac; done
[ -n "$config" ] && [ -n "$socket_name" ] || exit 3
cat "$config" > "$WTL_WESTON_CONFIG_LOG"
capture=$(sed -n 's/^path=//p' "$config")
DISPLAY=:88 XAUTHORITY="$XDG_RUNTIME_DIR/Xauthority" "$capture"
python3 -c 'import socket,sys,time; s=socket.socket(socket.AF_UNIX); s.bind(sys.argv[1]); print("ready",flush=True); time.sleep(30)' "$XDG_RUNTIME_DIR/$socket_name" &
socket_pid=$!
trap 'kill "$socket_pid" >/dev/null 2>&1 || true; wait "$socket_pid" >/dev/null 2>&1 || true; rm -f "$XDG_RUNTIME_DIR/$socket_name"' EXIT HUP INT TERM
if [ "${WTL_WESTON_CRASH_AFTER_SOCKET:-0}" = 1 ]; then sleep 0.3; exit 0; fi
wait "$socket_pid"
EOF
cat > "$TMP/fake-app" <<'EOF'
#!/bin/sh
printf 'DISPLAY=%s\n' "${DISPLAY:-}" >> "$WTL_APP_LOG"
printf 'XAUTHORITY=%s\n' "${XAUTHORITY:-}" >> "$WTL_APP_LOG"
printf 'WAYLAND_DISPLAY=%s\n' "${WAYLAND_DISPLAY:-}" >> "$WTL_APP_LOG"
printf 'XDG_SESSION_TYPE=%s\n' "${XDG_SESSION_TYPE:-}" >> "$WTL_APP_LOG"
printf 'QT_QPA_PLATFORM=%s\n' "${QT_QPA_PLATFORM:-}" >> "$WTL_APP_LOG"
printf 'QT_QPA_PLATFORM_PLUGIN_PATH=%s\n' "${QT_QPA_PLATFORM_PLUGIN_PATH:-}" >> "$WTL_APP_LOG"
printf 'QT_PLUGIN_PATH=%s\n' "${QT_PLUGIN_PATH:-}" >> "$WTL_APP_LOG"
printf '%s\n' "$*" >> "$WTL_APP_LOG"
if [ "${WTL_APP_HANG:-0}" = 1 ]; then exec sleep 30; fi
EOF
chmod +x "$fakebin/tty" "$fakebin/weston" "$TMP/fake-app"
: > "$runtime/Xauthority"
chmod 600 "$runtime/Xauthority"
export PATH="$fakebin:$PATH" XDG_RUNTIME_DIR="$runtime" WTL_BIN="$TMP/fake-app" WTL_WESTON_LOG="$TMP/weston.log" WTL_WESTON_CONFIG_LOG="$TMP/weston-config.log" WTL_APP_LOG="$TMP/app.log"
if env -u WTL_ENABLE_DRM_SESSION -u DISPLAY -u WAYLAND_DISPLAY XDG_SESSION_TYPE=tty sh "$ROOT/scripts/start-standalone-session.sh" >"$TMP/optin.log" 2>&1; then echo 'Standalone session did not require explicit opt-in.' >&2; exit 1; fi
grep -q 'WTL_ENABLE_DRM_SESSION=1' "$TMP/optin.log"
test ! -e "$WTL_WESTON_LOG"
if env -u WAYLAND_DISPLAY WTL_ENABLE_DRM_SESSION=1 DISPLAY=:host XDG_SESSION_TYPE=tty sh "$ROOT/scripts/start-standalone-session.sh" >"$TMP/host-display.log" 2>&1; then echo 'Standalone session accepted an active host display.' >&2; exit 1; fi
grep -q 'X11 display is active' "$TMP/host-display.log"
test ! -e "$WTL_WESTON_LOG"
chmod 755 "$runtime"
if env -u DISPLAY -u WAYLAND_DISPLAY WTL_ENABLE_DRM_SESSION=1 XDG_SESSION_TYPE=tty sh "$ROOT/scripts/start-standalone-session.sh" >"$TMP/unsafe-runtime.log" 2>&1; then echo 'Standalone session accepted a runtime directory accessible to other users.' >&2; exit 1; fi
grep -q 'accessible to other users' "$TMP/unsafe-runtime.log"
test ! -e "$WTL_WESTON_LOG"
chmod 700 "$runtime"
env -u DISPLAY -u WAYLAND_DISPLAY QT_QPA_PLATFORM=xcb QT_QPA_PLATFORM_PLUGIN_PATH=/host/qt/plugins QT_PLUGIN_PATH=/host/qt WTL_ENABLE_DRM_SESSION=1 XDG_SESSION_TYPE=tty sh "$ROOT/scripts/start-standalone-session.sh" >"$TMP/standalone.log" 2>&1
grep -q '^--backend=drm$' "$WTL_WESTON_LOG"
grep -q '^--tty=9$' "$WTL_WESTON_LOG"
grep -q '^--xwayland$' "$WTL_WESTON_LOG"
grep -q '^xwayland=true$' "$WTL_WESTON_CONFIG_LOG"
grep -q '^panel-position=top$' "$WTL_WESTON_CONFIG_LOG"
grep -q '^DISPLAY=:88$' "$WTL_APP_LOG"
grep -q "^XAUTHORITY=$runtime/Xauthority$" "$WTL_APP_LOG"
grep -q '^WAYLAND_DISPLAY=win-to-linux-session-' "$WTL_APP_LOG"
grep -q '^XDG_SESSION_TYPE=wayland$' "$WTL_APP_LOG"
grep -q '^QT_QPA_PLATFORM=$' "$WTL_APP_LOG"
grep -q '^QT_QPA_PLATFORM_PLUGIN_PATH=$' "$WTL_APP_LOG"
grep -q '^QT_PLUGIN_PATH=$' "$WTL_APP_LOG"
grep -q '^--maximized$' "$WTL_APP_LOG"
# A compositor crash after socket creation must terminate the supervised shell and return promptly.
rm -f "$WTL_WESTON_LOG" "$WTL_WESTON_CONFIG_LOG" "$WTL_APP_LOG"
if timeout 8 env -u DISPLAY -u WAYLAND_DISPLAY QT_QPA_PLATFORM=xcb QT_QPA_PLATFORM_PLUGIN_PATH=/host/qt/plugins QT_PLUGIN_PATH=/host/qt WTL_ENABLE_DRM_SESSION=1 WTL_WESTON_CRASH_AFTER_SOCKET=1 WTL_APP_HANG=1 XDG_SESSION_TYPE=tty sh "$ROOT/scripts/start-standalone-session.sh" >"$TMP/compositor-crash.log" 2>&1; then
    echo 'Standalone session did not report a compositor crash.' >&2
    exit 1
else
    crash_status=$?
    [ "$crash_status" -ne 124 ] || { echo 'Compositor crash supervision timed out.' >&2; exit 1; }
fi
grep -q 'Weston exited while Win-To-Linux was running' "$TMP/compositor-crash.log"
if find "$runtime" -maxdepth 1 -name 'win-to-linux-session-*' -print | grep -q .; then
    echo 'Standalone session left a private session artifact after compositor failure.' >&2
    exit 1
fi
printf '%s\n' 'Standalone-session preflight, launch, and compositor-crash supervision tests passed.'
