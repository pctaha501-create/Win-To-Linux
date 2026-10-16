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
# Simulate Weston failing before it creates its Wayland socket.
exit 7
EOF
cat > "$TMP/fake-app" <<'EOF'
#!/bin/sh
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
printf '%s\n' 'Standalone-session early-failure and cleanup regression passed.'
