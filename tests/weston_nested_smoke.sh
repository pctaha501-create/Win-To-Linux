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
[shell]
panel-position=none
locking=false
animation=none
EOF
unset WAYLAND_DISPLAY || true
weston_bin=$(command -v weston 2>/dev/null || true)
wayland_info=$(command -v wayland-info 2>/dev/null || true)
[ -n "$weston_bin" ] || { echo "Weston is required for this integration test." >&2; exit 1; }
[ -n "$wayland_info" ] || { echo "wayland-info is required for this integration test." >&2; exit 1; }
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
WAYLAND_DISPLAY=wayland-parent "$weston_bin" --backend=wayland --display=wayland-parent --socket=wayland-nested --renderer=pixman --config="$TMP/weston.ini" >"$TMP/nested.log" 2>&1 &
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
kill "$nested_pid" >/dev/null 2>&1 || true
wait "$nested_pid" >/dev/null 2>&1 || true
nested_pid=
kill "$parent_pid" >/dev/null 2>&1 || true
wait "$parent_pid" >/dev/null 2>&1 || true
parent_pid=
printf '%s\n' 'Nested Weston accepted a real Wayland client in headless mode.'
