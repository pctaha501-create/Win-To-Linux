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
grep -q '^ExecStart=%h/.local/bin/win-to-linux-isolated-preview$' "$UNIT"
grep -q '^Restart=on-failure$' "$UNIT"
grep -q '^RestartSec=5$' "$UNIT"
grep -q '^StartLimitIntervalSec=60$' "$UNIT"
grep -q '^StartLimitBurst=3$' "$UNIT"
grep -q '^After=graphical-session.target$' "$UNIT"
grep -q 'packaging/systemd/user/Win-To-Linux.service' "$INSTALLER"
grep -q 'install -m 0644' "$INSTALLER"
grep -q 'Win-To-Linux.service' "$UNINSTALLER"
for script in "$ROOT"/scripts/*.sh; do sh -n "$script"; done
printf '%s\n' 'Session service packaging checks passed.'
