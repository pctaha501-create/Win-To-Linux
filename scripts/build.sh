#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="${BUILD_DIR:-"$ROOT/build"}"
cmake -S "$ROOT" -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build "$BUILD_DIR" --parallel "${BUILD_JOBS:-2}"
ctest --test-dir "$BUILD_DIR" --output-on-failure
