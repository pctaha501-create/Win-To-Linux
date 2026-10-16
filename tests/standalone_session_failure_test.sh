#!/usr/bin/env sh
set -eu
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
config= socket_name=
for arg in "$@"; do case "$arg" in --config=*) config=${arg#--config=} ;; --socket=*) socket_name=${arg#--socket=} ;; esac; done
[ -n "$config" ] && [ -n "$socket_name" ] || exit 3
if [ "${WTL_FAKE_MODE:-early}" != crash ]; then
  # Simulate Weston failing before it creates its Wayland socket.
  exit 7
fi
capture=$(sed -n 's/^path=//p' "$config")
DISPLAY=:88 XAUTHORITY="$XDG_RUNTIME_DIR/Xauthority" "$capture"
# Publish a real Unix socket briefly, then simulate a compositor crash.
python3 -c 'import socket,sys,time; s=socket.socket(socket.AF_UNIX); s.bind(sys.argv[1]); s.listen(1); time.sleep(0.7)' "$XDG_RUNTIME_DIR/$socket_name"
exit 7
EOF
cat > "$TMP/fake-app" <<'EOF'
#!/bin/sh
if [ "${WTL_FAKE_MODE:-early}" = crash ]; then
  trap 'exit 0' TERM INT
  while :; do sleep 1; done
fi
exit 99
EOF
chmod +x "$fakebin/tty" "$fakebin/weston" "$TMP/fake-app"
export PATH="$fakebin:$PATH" XDG_RUNTIME_DIR="$runtime" WTL_BIN="$TMP/fake-app" WTL_WESTON_LOG="$TMP/weston.log"
start=$(date +%s)
if env -u DISPLAY -u WAYLAND_DISPLAY WTL_ENABLE_DRM_SESSION=1 XDG_SESSION_TYPE=tty sh "$ROOT/scripts/start-standalone-session.sh" >"$TMP/launcher.log" 2>&1; then
  echo 'Launcher unexpectedly accepted Weston exiting before socket creation.' >&2; exit 1
fi
elapsed=$(( $(date +%s) - start ))
[ "$elapsed" -lt 4 ] || { echo "Early Weston failure took too long: ${elapsed}s" >&2; cat "$TMP/launcher.log" >&2; exit 1; }
grep -q 'Weston DRM did not create its Wayland socket' "$TMP/launcher.log"
test -s "$WTL_WESTON_LOG"
# All per-run private files are removed by the launcher's EXIT trap.
if find "$runtime" -maxdepth 1 -type f -name 'win-to-linux-session.*' -o -name 'win-to-linux-xwayland-*' | grep -q .; then
  echo 'Standalone launcher leaked private temporary files after Weston failure.' >&2
  find "$runtime" -maxdepth 1 -type f -print >&2
  exit 1
fi
test ! -e "$runtime/win-to-linux-session-$(id -u)-$$"
# Now simulate a compositor crash after the socket exists while the shell child is still running.
start=$(date +%s)
if env -u DISPLAY -u WAYLAND_DISPLAY WTL_FAKE_MODE=crash WTL_ENABLE_DRM_SESSION=1 XDG_SESSION_TYPE=tty sh "$ROOT/scripts/start-standalone-session.sh" >"$TMP/crash.log" 2>&1; then
  echo 'Launcher did not report the compositor crash.' >&2; exit 1
fi
elapsed=$(( $(date +%s) - start ))
[ "$elapsed" -lt 5 ] || { echo "Compositor watchdog took too long: ${elapsed}s" >&2; cat "$TMP/crash.log" >&2; exit 1; }
grep -q 'Weston exited while Win-To-Linux was running' "$TMP/crash.log"
if find "$runtime" -maxdepth 1 -type f \( -name 'win-to-linux-session.*' -o -name 'win-to-linux-xwayland-*' -o -name 'win-to-linux-watchdog-*' \) | grep -q .; then
  echo 'Standalone launcher leaked private temporary files after compositor crash.' >&2
  find "$runtime" -maxdepth 1 -type f -print >&2
  exit 1
fi
printf '%s\n' 'Standalone-session early-failure, compositor-watchdog, and cleanup regressions passed.'
