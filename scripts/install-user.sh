#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="${BUILD_DIR:-"$ROOT/build"}"
PREFIX="${PREFIX:-"$HOME/.local"}"
if [ ! -x "$BUILD_DIR/src/app/win-to-linux" ]; then
  echo "Build not found; building and testing first."
  "$ROOT/scripts/build.sh"
fi
cmake --install "$BUILD_DIR" --prefix "$PREFIX"
echo "Installed Win-To-Linux for the current user in $PREFIX."
echo "Run it with: $PREFIX/bin/win-to-linux"
