#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT="${1:-$ROOT/os/manifests/host-$(date -u +%Y%m%dT%H%M%SZ)}"
mkdir -p "$OUT"
for tool in uname dpkg-query apt-cache rpm dnf pacman zypper apk xbps-install gcc g++ cmake ninja git grub-mkrescue xorriso mksquashfs; do
  if command -v "$tool" >/dev/null 2>&1; then printf '%s\t%s\n' "$tool" "$(command -v "$tool")"; else printf '%s\tMISSING\n' "$tool"; fi
done > "$OUT/tools.tsv"
uname -a > "$OUT/uname.txt"
cat /etc/os-release > "$OUT/os-release"
if command -v dpkg-query >/dev/null 2>&1; then
  dpkg-query -W -f='${binary:Package}\t${Version}\t${Architecture}\n' 2>/dev/null | sort > "$OUT/dpkg-packages.tsv"
fi
if command -v rpm >/dev/null 2>&1; then
  rpm -qa --qf '%{NAME}\t%{VERSION}-%{RELEASE}\t%{ARCH}\n' 2>/dev/null | sort > "$OUT/rpm-packages.tsv"
fi
if [[ -r /proc/config.gz ]]; then zcat /proc/config.gz > "$OUT/kernel-config"; elif [[ -r "/boot/config-$(uname -r)" ]]; then cp "/boot/config-$(uname -r)" "$OUT/kernel-config"; fi
find /usr/bin /usr/sbin /bin /sbin -maxdepth 1 -type f -executable -printf '%f\t%p\n' 2>/dev/null | sort -u > "$OUT/executables.tsv"
printf 'Inventory written to %s\n' "$OUT"
