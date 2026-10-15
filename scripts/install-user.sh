#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="${BUILD_DIR:-"$ROOT/build"}"
PREFIX="${PREFIX:-"$HOME/.local"}"
echo "Building and testing the current source before installation."
sh "$ROOT/scripts/build.sh"
cmake --install "$BUILD_DIR" --prefix "$PREFIX"
echo "Installed Win-To-Linux for the current user in $PREFIX."
echo "Run it with: $PREFIX/bin/win-to-linux"
