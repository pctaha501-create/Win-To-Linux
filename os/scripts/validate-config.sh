#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
for manifest in "$ROOT"/os/config/packages-*.txt; do
  [[ -f "$manifest" ]] || { echo "Missing package manifest: $manifest" >&2; exit 1; }
  awk 'NF && $1 !~ /^#/ { if ($1 ~ /[^A-Za-z0-9+_.:-]/) { print "Invalid package token: " $1 > "/dev/stderr"; bad=1 } if (seen[$1]++) { print "Duplicate package: " $1 > "/dev/stderr"; bad=1 } count++ } END { if (!count) { print "Empty package manifest" > "/dev/stderr"; bad=1 } exit bad }' "$manifest"
done
for script in "$ROOT"/os/scripts/*.sh; do bash -n "$script"; done
echo 'PASS: OS package manifests and shell scripts are valid'
