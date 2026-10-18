#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
UNIT="$ROOT/packaging/systemd/user/Win-To-Linux.service"
INSTALLER="$ROOT/scripts/install-user.sh"
UNINSTALLER="$ROOT/scripts/uninstall-user.sh"
[ -f "$UNIT" ]
grep -q '^\[Unit\]$' "$UNIT"
grep -q '^\[Service\]$' "$UNIT"
grep -q '^Type=simple$' "$UNIT"
grep -Fq 'ExecStart="%h/.local/bin/win-to-linux-isolated-preview"' "$UNIT"
grep -q -- '--preview-check' "$ROOT/scripts/start-isolated-preview.sh"
grep -q '^Restart=on-failure$' "$UNIT"
grep -q '^RestartSec=5$' "$UNIT"
grep -q '^StartLimitIntervalSec=60$' "$UNIT"
grep -q '^StartLimitBurst=3$' "$UNIT"
grep -q '^After=graphical-session.target$' "$UNIT"
grep -q 'packaging/systemd/user/Win-To-Linux.service' "$INSTALLER"
grep -q 'render-user-service.sh' "$INSTALLER"
grep -q 'Win-To-Linux.service' "$UNINSTALLER"
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
prefix="$tmpdir/custom install prefix"
sh "$ROOT/scripts/render-user-service.sh" "$UNIT" "$tmpdir/custom.service" "$prefix"
grep -Fqx "Environment=\"WTL_BIN=$prefix/bin/win-to-linux\"" "$tmpdir/custom.service"
grep -Fqx "ExecStart=\"$prefix/bin/win-to-linux-isolated-preview\"" "$tmpdir/custom.service"
for script in "$ROOT"/scripts/*.sh; do sh -n "$script"; done
printf '%s\n' 'Session service packaging checks passed.'
