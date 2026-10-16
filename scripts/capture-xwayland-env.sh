#!/bin/sh
# Record only the DISPLAY and XAUTHORITY that Weston exports to its own autolaunch child.
set -eu
output=${WTL_XWAYLAND_ENV_FILE:-}
[ -n "$output" ] || exit 0
runtime=${XDG_RUNTIME_DIR:-}
[ -n "$runtime" ] || exit 2
case "$output" in "$runtime"/win-to-linux-xwayland-env.*) ;; *) exit 2 ;; esac
umask 077
tmp_file="$output.tmp.$$"
trap 'rm -f -- "$tmp_file"' EXIT HUP INT TERM
{
    printf '%s\n' "${DISPLAY:-}"
    printf '%s\n' "${XAUTHORITY:-}"
} > "$tmp_file"
chmod 600 "$tmp_file"
mv -f -- "$tmp_file" "$output"
trap - EXIT HUP INT TERM
