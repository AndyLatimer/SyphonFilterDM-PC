#!/usr/bin/env bash
# Builds the Linux binaries and runs the unit tests.
# Usage: scripts/build_linux.sh [/path/to/your/SyphonFilterDarkMirror.iso]  (without it: runtime, tools and tests only)
set -euo pipefail
cd "$(dirname "$0")/.."
cmake -S . -B build/linux -DCMAKE_BUILD_TYPE=Release ${1:+-DSF_GAME_ISO=$(realpath "$1")} ${SDL2_DIR:+-DSDL2_DIR=$SDL2_DIR}
cmake --build build/linux -j"$(nproc)"
ctest --test-dir build/linux --output-on-failure
