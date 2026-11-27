#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
command -v dpkg-deb >/dev/null 2>&1 || { echo 'SKIP: dpkg-deb unavailable'; exit 0; }
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
mkdir -p "$tmp/pkg/DEBIAN"
cat > "$tmp/pkg/DEBIAN/control" <<'CONTROL'
Package: wtl-package-probe
Version: 1.0
Architecture: all
Maintainer: Win-To-Linux Build <build@example.invalid>
Description: Temporary package metadata probe
CONTROL
dpkg-deb --build "$tmp/pkg" "$tmp/probe.deb" >/dev/null
result="$("$ROOT/os/scripts/inspect-package.sh" "$tmp/probe.deb")"
grep -q '^format=deb$' <<< "$result"
grep -q '^Package: wtl-package-probe$' <<< "$result"
grep -q '^Version: 1.0$' <<< "$result"
echo 'PASS: DEB package metadata inspection'
if ! command -v rpm >/dev/null 2>&1; then echo 'INFO: RPM inspection requires rpm tooling; not present on Ubuntu build host'; fi
