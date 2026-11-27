#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TARGET="${1:-$ROOT/out/rootfs}"
fail(){ echo "FAIL: $*" >&2; exit 1; }
[[ -d "$TARGET" ]] || fail "rootfs directory missing: $TARGET"
for p in etc/os-release etc/hostname etc/hosts etc/fstab etc/machine-id var/lib/dpkg/status var/lib/win-to-linux-package-manifest.tsv var/lib/win-to-linux-build.json; do [[ -s "$TARGET/$p" || "$p" == etc/machine-id ]] || fail "missing target file: $p"; done
for p in bin/bash usr/bin/apt-get usr/bin/dpkg-query usr/lib/systemd/systemd; do [[ -x "$TARGET/$p" ]] || fail "missing executable: $p"; done
for p in dev proc sys run tmp; do [[ -d "$TARGET/$p" ]] || fail "missing mountpoint: /$p"; done
[[ "$(stat -c %a "$TARGET/tmp")" == 1777 ]] || fail "/tmp must be mode 1777"
grep -q '^ID=win-to-linux$' "$TARGET/etc/os-release" || fail "wrong os-release ID"
dpkg-query --admindir="$TARGET/var/lib/dpkg" -W systemd apt dpkg >/dev/null || fail "target package database is incomplete"
echo "PASS: standalone rootfs structure and package database ($TARGET)"
