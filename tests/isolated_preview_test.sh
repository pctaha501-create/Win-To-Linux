#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TMP=$(mktemp -d)
socket_pid=
cleanup() {
    if [ -n "$socket_pid" ]; then kill "$socket_pid" >/dev/null 2>&1 || true; wait "$socket_pid" >/dev/null 2>&1 || true; fi
    rm -rf -- "$TMP"
}
trap cleanup EXIT HUP INT TERM
runtime="$TMP/runtime"
fakebin="$TMP/bin"
mkdir -m 700 "$runtime" "$fakebin"
socket_path="$runtime/wayland-test"
python3 -c 'import socket,sys,time; s=socket.socket(socket.AF_UNIX); s.bind(sys.argv[1]); print("ready",flush=True); time.sleep(30)' "$socket_path" >"$TMP/socket.log" 2>&1 &
socket_pid=$!
i=0
while [ ! -S "$socket_path" ] && [ "$i" -lt 50 ]; do sleep 0.1; i=$((i+1)); done
[ -S "$socket_path" ] || { cat "$TMP/socket.log" >&2; echo "Fake Wayland socket was not created." >&2; exit 1; }
cat > "$fakebin/cage" <<'EOF'
#!/bin/sh
printf '%s\n' "$*" > "$WTL_CAGE_LOG"
[ "${1:-}" = "--" ] && shift
WAYLAND_DISPLAY=wayland-cage "$@"
EOF
cat > "$TMP/fake-app" <<'EOF'
#!/bin/sh
printf 'WAYLAND_DISPLAY=%s\n' "${WAYLAND_DISPLAY:-}" >> "$WTL_APP_LOG"
printf 'DISPLAY=%s\n' "${DISPLAY:-}" >> "$WTL_APP_LOG"
printf 'XAUTHORITY=%s\n' "${XAUTHORITY:-}" >> "$WTL_APP_LOG"
printf 'QT_QPA_PLATFORM=%s\n' "${QT_QPA_PLATFORM:-}" >> "$WTL_APP_LOG"
printf 'XDG_SESSION_TYPE=%s\n' "${XDG_SESSION_TYPE:-}" >> "$WTL_APP_LOG"
printf '%s\n' "$*" >> "$WTL_APP_LOG"
if [ "${1:-}" = "--preview-check" ]; then
    [ "${WTL_FAIL_PREFLIGHT:-0}" != 1 ] && exit 0
    exit 2
fi
exit 0
EOF
chmod +x "$fakebin/cage" "$TMP/fake-app"
export PATH="$fakebin:$PATH" XDG_RUNTIME_DIR="$runtime" WAYLAND_DISPLAY=wayland-test DISPLAY=:host-display XAUTHORITY=/host/auth QT_QPA_PLATFORM=xcb WTL_BIN="$TMP/fake-app" WTL_CAGE_LOG="$TMP/cage.log" WTL_APP_LOG="$TMP/app.log"
sh "$ROOT/scripts/start-isolated-preview.sh"
grep -q '^--preview-check$' "$WTL_APP_LOG"
grep -q "^-- $WTL_BIN --fullscreen$" "$WTL_CAGE_LOG"
rm -f "$WTL_CAGE_LOG" "$WTL_APP_LOG"
cat > "$fakebin/weston" <<'EOF'
#!/bin/sh
printf '%s\n' "$@" > "$WTL_WESTON_LOG"
for arg in "$@"; do case "$arg" in --config=*) cat "${arg#--config=}" > "$WTL_WESTON_CONFIG_LOG" ;; esac; done
if [ "${WTL_WESTON_FAIL:-0}" = 1 ]; then echo 'simulated unsupported nested backend' >&2; exit 1; fi
socket_name=
for arg in "$@"; do case "$arg" in --socket=*) socket_name=${arg#--socket=} ;; esac; done
[ -n "$socket_name" ] || exit 3
python3 -c 'import socket,sys,time; s=socket.socket(socket.AF_UNIX); s.bind(sys.argv[1]); print("ready",flush=True); time.sleep(30)' "$XDG_RUNTIME_DIR/$socket_name" &
socket_pid=$!
trap 'kill "$socket_pid" >/dev/null 2>&1 || true; wait "$socket_pid" >/dev/null 2>&1 || true; rm -f "$XDG_RUNTIME_DIR/$socket_name"' EXIT HUP INT TERM
wait "$socket_pid"
EOF
chmod +x "$fakebin/weston"
export WTL_WESTON_LOG="$TMP/weston.log" WTL_WESTON_CONFIG_LOG="$TMP/weston-config.log"
sh "$ROOT/scripts/start-isolated-preview.sh"
grep -q '^--backend=wayland$' "$WTL_WESTON_LOG"
grep -q '^--display=wayland-test$' "$WTL_WESTON_LOG"
grep -q "^--socket=win-to-linux-$(id -u)-" "$WTL_WESTON_LOG"
grep -q '^--renderer=pixman$' "$WTL_WESTON_LOG"
grep -q '^--renderer=pixman$' "$WTL_WESTON_LOG"
grep -q '^panel-position=none$' "$WTL_WESTON_CONFIG_LOG"
grep -q '^shell=desktop$' "$WTL_WESTON_CONFIG_LOG"
grep -q '^--maximized$' "$WTL_APP_LOG"
grep -q "^WAYLAND_DISPLAY=win-to-linux-$(id -u)-" "$WTL_APP_LOG"
test ! -e "$WTL_CAGE_LOG"
rm -f "$WTL_WESTON_LOG" "$WTL_WESTON_CONFIG_LOG" "$WTL_APP_LOG"
if WTL_FAIL_PREFLIGHT=1 sh "$ROOT/scripts/start-isolated-preview.sh" >"$TMP/refusal.log" 2>&1; then
    echo "Launcher accepted a failed readiness preflight." >&2
    exit 1
fi
grep -q 'readiness preflight refused to launch' "$TMP/refusal.log"
test ! -e "$WTL_CAGE_LOG"
test ! -e "$WTL_WESTON_LOG"
test ! -e "$WTL_WESTON_CONFIG_LOG"
printf '%s\n' 'Isolated preview launch/preflight integration checks passed.'
