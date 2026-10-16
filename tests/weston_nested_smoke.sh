#!/usr/bin/env sh
set -eu
TMP=$(mktemp -d)
parent_pid=
nested_pid=
cleanup() {
    if [ -n "$nested_pid" ]; then kill "$nested_pid" >/dev/null 2>&1 || true; wait "$nested_pid" >/dev/null 2>&1 || true; fi
    if [ -n "$parent_pid" ]; then kill "$parent_pid" >/dev/null 2>&1 || true; wait "$parent_pid" >/dev/null 2>&1 || true; fi
    rm -rf -- "$TMP"
}
trap cleanup EXIT HUP INT TERM
runtime="$TMP/runtime"
mkdir -m 700 "$runtime"
export XDG_RUNTIME_DIR="$runtime"
cat > "$TMP/weston.ini" <<'EOF'
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
EOF
unset WAYLAND_DISPLAY || true
weston_bin=$(command -v weston 2>/dev/null || true)
wayland_info=$(command -v wayland-info 2>/dev/null || true)
[ -n "$weston_bin" ] || { echo "Weston is required for this integration test." >&2; exit 1; }
[ -n "$wayland_info" ] || { echo "wayland-info is required for this integration test." >&2; exit 1; }
xdpyinfo_bin=$(command -v xdpyinfo 2>/dev/null || true)
[ -n "$xdpyinfo_bin" ] || { echo "xdpyinfo (x11-utils) is required for the XWayland integration test." >&2; exit 1; }
xauth_bin=$(command -v xauth 2>/dev/null || true)
[ -n "$xauth_bin" ] || { echo "xauth is required for the XWayland integration test." >&2; exit 1; }
"$weston_bin" --backend=headless --socket=wayland-parent --renderer=pixman --no-config >"$TMP/parent.log" 2>&1 &
parent_pid=$!
i=0
while [ ! -S "$runtime/wayland-parent" ] && [ "$i" -lt 100 ]; do
    kill -0 "$parent_pid" 2>/dev/null || break
    sleep 0.1
    i=$((i+1))
done
if [ ! -S "$runtime/wayland-parent" ]; then
    cat "$TMP/parent.log" >&2
    echo "Headless Weston parent did not create its Wayland socket." >&2
    exit 1
fi
WAYLAND_DISPLAY=wayland-parent "$weston_bin" --backend=wayland --display=wayland-parent --socket=wayland-nested --renderer=pixman --xwayland --config="$TMP/weston.ini" >"$TMP/nested.log" 2>&1 &
nested_pid=$!
i=0
while [ ! -S "$runtime/wayland-nested" ] && [ "$i" -lt 100 ]; do
    kill -0 "$nested_pid" 2>/dev/null || break
    sleep 0.1
    i=$((i+1))
done
if [ ! -S "$runtime/wayland-nested" ]; then
    cat "$TMP/nested.log" >&2
    echo "Nested Weston did not create its unique Wayland socket." >&2
    exit 1
fi
if ! WAYLAND_DISPLAY=wayland-nested "$wayland_info" >"$TMP/client.log" 2>&1; then
    cat "$TMP/nested.log" "$TMP/client.log" >&2
    echo "A Wayland client could not connect to nested Weston." >&2
    exit 1
fi
grep -q 'wl_compositor' "$TMP/client.log" || { cat "$TMP/client.log" >&2; exit 1; }
# Wait for Weston to publish its private XWayland display. This only runs inside the throwaway test runtime.
i=0
while ! grep -q 'xserver listening on display :' "$TMP/nested.log" && [ "$i" -lt 100 ]; do
    kill -0 "$nested_pid" 2>/dev/null || break
    sleep 0.1
    i=$((i+1))
done
xdisplay=$(sed -n 's/.*xserver listening on display \(:[0-9][0-9]*\).*/\1/p' "$TMP/nested.log" | head -n 1)
if [ -z "$xdisplay" ]; then
    cat "$TMP/nested.log" >&2
    echo "Nested Weston did not expose an XWayland display." >&2
    exit 1
fi
# Weston may keep its cookie in a runtime file. Try only private files under this test's disposable directories.
xclient_ok=0
for auth_file in $(find "$TMP" "$runtime" -type f \( -iname '*auth*' -o -name 'Xauthority' \) -print 2>/dev/null); do
    if DISPLAY="$xdisplay" XAUTHORITY="$auth_file" "$xdpyinfo_bin" >"$TMP/x11-client.log" 2>&1; then xclient_ok=1; break; fi
done
if [ "$xclient_ok" -ne 1 ]; then
    cat "$TMP/nested.log" "$TMP/x11-client.log" >&2
    echo "XWayland published $xdisplay but an X11 client could not authenticate using the nested session's private authority files." >&2
    exit 1
fi
grep -q 'name of display' "$TMP/x11-client.log" || { cat "$TMP/x11-client.log" >&2; exit 1; }
kill "$nested_pid" >/dev/null 2>&1 || true
wait "$nested_pid" >/dev/null 2>&1 || true
nested_pid=
kill "$parent_pid" >/dev/null 2>&1 || true
wait "$parent_pid" >/dev/null 2>&1 || true
parent_pid=
printf '%s\n' 'Nested Weston accepted a real Wayland client in headless mode.'
