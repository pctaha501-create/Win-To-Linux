#!/usr/bin/env bash
set -Eeuo pipefail
if [[ $# -ne 1 || ! -f "$1" ]]; then echo "Usage: $0 package.deb|package.rpm" >&2; exit 2; fi
file="$(realpath "$1")"
case "$file" in
  *.deb)
    command -v dpkg-deb >/dev/null 2>&1 || { echo 'dpkg-deb is required to inspect DEB packages' >&2; exit 3; }
    printf 'format=deb\n'
    dpkg-deb --field "$file" Package Version Architecture Depends Pre-Depends Conflicts Replaces
    ;;
  *.rpm)
    command -v rpm >/dev/null 2>&1 || { echo 'rpm tooling is not available on this build host; RPM inspection is deferred to the RPM-enabled CI/runtime image' >&2; exit 3; }
    printf 'format=rpm\n'
    rpm -qp --queryformat 'Package: %{NAME}\nVersion: %{VERSION}-%{RELEASE}\nArchitecture: %{ARCH}\nRequires: %{REQUIRENAME}\n' "$file"
    ;;
  *) echo 'Unsupported package extension; expected .deb or .rpm' >&2; exit 2 ;;
esac
