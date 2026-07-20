#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
BUILD_DIR="${BUILD_DIR:-"$ROOT_DIR/.build/native"}"

cmake -G Ninja -S "$ROOT_DIR" -B "$BUILD_DIR" -DDHTML_BUILD_BENCHMARKS=ON
cmake --build "$BUILD_DIR" --parallel
"$BUILD_DIR/tel-json-benchmark" "[!benchmark]"
