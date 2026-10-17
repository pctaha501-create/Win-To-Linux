#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
FAKE="$TMP/fake-winux11"
cat >"$FAKE" <<'FAKE_EOF'
#!/bin/sh
case "${1:-}" in
  --version) echo 'Win-To-Linux 0.2.0' ;;
  --session-check) echo 'Session: test-session' ;;
  --diagnostic-report) echo 'Win-To-Linux diagnostic report'; echo 'Runtime check: test';;
  --preview-check)
    if [ "${FAKE_PREVIEW_READY:-0}" = 1 ]; then echo 'Preview readiness: ready'; exit 0; fi
    echo 'Not ready for nested preview: no user Wayland socket'; exit 2 ;;
  *) echo "Unexpected argument: ${1:-}" >&2; exit 90 ;;
esac
FAKE_EOF
chmod +x "$FAKE"
WTL_BIN="$FAKE" sh "$ROOT/scripts/verify-vm.sh" >"$TMP/blocked.log" 2>&1
grep -q 'Win-To-Linux VM diagnostics (read-only)' "$TMP/blocked.log"
grep -q 'Win-To-Linux 0.2.0' "$TMP/blocked.log"
grep -q 'Session: test-session' "$TMP/blocked.log"
grep -q 'Win-To-Linux diagnostic report' "$TMP/blocked.log"
grep -q 'RESULT: Preview is blocked' "$TMP/blocked.log"
FAKE_PREVIEW_READY=1 WTL_BIN="$FAKE" sh "$ROOT/scripts/verify-vm.sh" >"$TMP/ready.log" 2>&1
grep -q 'RESULT: Preview preflight is ready' "$TMP/ready.log"
printf '%s\n' 'VM verifier tests passed (blocked and ready paths).'
