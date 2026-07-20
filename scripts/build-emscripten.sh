#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
BUILD_DIR="${BUILD_DIR:-"$ROOT_DIR/.build/emscripten"}"

emcmake cmake -G Ninja -S "$ROOT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" --parallel
