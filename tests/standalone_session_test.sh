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
wait "$socket_pid"
EOF
cat > "$TMP/fake-app" <<'EOF'
#!/bin/sh
printf 'DISPLAY=%s\n' "${DISPLAY:-}" >> "$WTL_APP_LOG"
printf 'XAUTHORITY=%s\n' "${XAUTHORITY:-}" >> "$WTL_APP_LOG"
printf 'WAYLAND_DISPLAY=%s\n' "${WAYLAND_DISPLAY:-}" >> "$WTL_APP_LOG"
printf 'XDG_SESSION_TYPE=%s\n' "${XDG_SESSION_TYPE:-}" >> "$WTL_APP_LOG"
printf '%s\n' "$*" >> "$WTL_APP_LOG"
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
env -u DISPLAY -u WAYLAND_DISPLAY WTL_ENABLE_DRM_SESSION=1 XDG_SESSION_TYPE=tty sh "$ROOT/scripts/start-standalone-session.sh" >"$TMP/standalone.log" 2>&1
grep -q '^--backend=drm$' "$WTL_WESTON_LOG"
grep -q '^--tty=9$' "$WTL_WESTON_LOG"
grep -q '^--xwayland$' "$WTL_WESTON_LOG"
grep -q '^xwayland=true$' "$WTL_WESTON_CONFIG_LOG"
grep -q '^panel-position=top$' "$WTL_WESTON_CONFIG_LOG"
grep -q '^DISPLAY=:88$' "$WTL_APP_LOG"
grep -q "^XAUTHORITY=$runtime/Xauthority$" "$WTL_APP_LOG"
grep -q '^WAYLAND_DISPLAY=win-to-linux-session-' "$WTL_APP_LOG"
grep -q '^XDG_SESSION_TYPE=wayland$' "$WTL_APP_LOG"
grep -q '^--fullscreen$' "$WTL_APP_LOG"
printf '%s\n' 'Standalone-session preflight and supervised-launch tests passed.'
