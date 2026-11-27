#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SUITE="${WTL_SUITE:-noble}"
ARCH="${WTL_ARCH:-$(dpkg --print-architecture)}"
MIRROR="${WTL_MIRROR:-http://archive.ubuntu.com/ubuntu}"
OUT="$ROOT/out"
TARGET="$OUT/rootfs"
if [[ "${1:-}" == "--help" ]]; then
  echo "Usage: sudo $0"; exit 0
fi
[[ $EUID -eq 0 ]] || { echo "Run as root." >&2; exit 2; }
command -v mmdebstrap >/dev/null 2>&1 || { echo "mmdebstrap required; use the CI rootfs job. Host packages are never installed by this script." >&2; exit 3; }
[[ "$SUITE" =~ ^[a-z][a-z0-9.-]+$ && "$ARCH" =~ ^[a-z0-9_-]+$ ]] || { echo "Invalid suite or architecture." >&2; exit 2; }
mkdir -p "$OUT"
[[ ! -e "$TARGET" ]] || { echo "Refusing to overwrite $TARGET." >&2; exit 4; }
packages=()
for file in "$ROOT/os/config/packages-common.txt" "$ROOT/os/config/packages-deb.txt"; do
  while IFS= read -r p; do
    [[ -z "$p" || "$p" == \#* ]] || packages+=("$p")
  done < "$file"
done
include="$(IFS=,; echo "${packages[*]}")"
mkdir "$TARGET"
rmdir "$TARGET"
mmdebstrap --variant=minbase --architectures="$ARCH" --components=main,restricted,universe,multiverse --include="$include" "$SUITE" "$TARGET" "$MIRROR"
printf 'win-to-linux\n' > "$TARGET/etc/hostname"
cat > "$TARGET/etc/hosts" <<'HOSTS'
127.0.0.1 localhost
127.0.1.1 win-to-linux
::1 localhost ip6-localhost ip6-loopback
HOSTS
: > "$TARGET/etc/machine-id"
mkdir -p "$TARGET/var/lib/dbus" "$TARGET/dev" "$TARGET/proc" "$TARGET/sys" "$TARGET/run" "$TARGET/tmp" "$TARGET/boot"
chmod 1777 "$TARGET/tmp"
cat > "$TARGET/etc/fstab" <<'FSTAB'
# Installer must populate target UUIDs.
# <file system> <mount point> <type> <options> <dump> <pass>
FSTAB
cat > "$TARGET/etc/os-release" <<'OSREL'
NAME="Win-To-Linux"
PRETTY_NAME="Win-To-Linux 0.1 (Noble base)"
ID=win-to-linux
ID_LIKE="ubuntu debian"
VERSION_ID="0.1"
VERSION_CODENAME=noble
VERSION="0.1"
HOME_URL="https://github.com/pctaha501-create/Win-To-Linux"
OSREL
chroot "$TARGET" /usr/bin/apt-get clean
chroot "$TARGET" /usr/bin/dpkg-query -W -f='${binary:Package}\t${Version}\t${Architecture}\n' | sort > "$TARGET/var/lib/win-to-linux-package-manifest.tsv"
python3 - "$TARGET" "$SUITE" "$ARCH" "$MIRROR" <<'PY'
import json,pathlib,sys,datetime
root=pathlib.Path(sys.argv[1]); suite,arch,mirror=sys.argv[2:]
pkgs=[]
for line in (root/"var/lib/win-to-linux-package-manifest.tsv").read_text().splitlines():
 f=line.split("\t")
 if len(f)==3: pkgs.append({"name":f[0],"version":f[1],"architecture":f[2]})
d={"distribution":"Win-To-Linux","version":"0.1","base":"Ubuntu","suite":suite,"architecture":arch,"mirror":mirror,"built_at_utc":datetime.datetime.now(datetime.timezone.utc).isoformat(),"package_count":len(pkgs),"packages":pkgs}
(root/"var/lib/win-to-linux-build.json").write_text(json.dumps(d,indent=2)+"\n")
PY
echo "Built $TARGET ($SUITE/$ARCH)"

./os/scripts/test-rootfs.sh "$TARGET"
