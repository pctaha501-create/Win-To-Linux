#!/usr/bin/env sh
set -eu
[ "$#" -eq 3 ] || { echo "Usage: render-user-service.sh SOURCE DESTINATION PREFIX" >&2; exit 2; }
SOURCE=$1
DESTINATION=$2
PREFIX=$3
[ -f "$SOURCE" ] || { echo "Missing service template: $SOURCE" >&2; exit 1; }
[ -n "$PREFIX" ] || { echo "Install prefix must not be empty." >&2; exit 1; }
# Escape sed replacement metacharacters while preserving spaces in the path.
PREFIX_BIN=$(printf '%s' "$PREFIX/bin" | sed 's/[\\&|]/\\&/g')
sed "s|%h/.local/bin|$PREFIX_BIN|g" "$SOURCE" > "$DESTINATION"
chmod 0644 "$DESTINATION"
