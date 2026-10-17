#!/usr/bin/env sh
# Read-only VM acceptance diagnostics. Does not install packages or alter the display manager.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
if [ -n "${WTL_BIN:-}" ]; then
    BIN=$WTL_BIN
elif command -v Winux11 >/dev/null 2>&1; then
    BIN=$(command -v Winux11)
elif command -v win-to-linux >/dev/null 2>&1; then
    BIN=$(command -v win-to-linux)
elif [ -x "$ROOT/build/src/app/win-to-linux" ]; then
    BIN="$ROOT/build/src/app/win-to-linux"
else
    echo "Win-To-Linux executable not found. Build/install it first or set WTL_BIN." >&2
    exit 1
fi
[ -x "$BIN" ] || { echo "Executable is not runnable: $BIN" >&2; exit 1; }
printf '%s\n' '=== Win-To-Linux VM diagnostics (read-only) ===' "Executable: $BIN" '' '--- Version ---'
"$BIN" --version
printf '%s\n' '' '--- Host and session ---'
"$BIN" --session-check
printf '%s\n' '' '--- Support report ---'
"$BIN" --diagnostic-report
printf '%s\n' '' '--- Nested preview readiness ---'
set +e
"$BIN" --preview-check
rc=$?
set -e
if [ "$rc" -eq 0 ]; then
    printf '%s\n' '' 'RESULT: Preview preflight is ready. This does not replace the manual keyboard/focus/accessibility test.'
elif [ "$rc" -eq 2 ]; then
    printf '%s\n' '' 'RESULT: Preview is blocked by one or more reported prerequisites. The existing desktop was not changed.'
else
    printf 'RESULT: Unexpected preview-check exit code %s.\n' "$rc" >&2
    exit "$rc"
fi
