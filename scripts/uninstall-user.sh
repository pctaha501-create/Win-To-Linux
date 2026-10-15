#!/usr/bin/env sh
set -eu
PREFIX="${PREFIX:-"$HOME/.local"}"
rm -f -- "$PREFIX/bin/win-to-linux" "$PREFIX/bin/win-to-linux-isolated-preview" "$PREFIX/share/applications/win-to-linux.desktop"
echo "Removed the Win-To-Linux executable, isolated-preview launcher, and desktop launcher from $PREFIX."
echo "User settings and build files were left untouched."
