#!/usr/bin/env bash
# Cross-compiles the Windows build and packages dist/SyphonFilterDarkMirror-win64.zip.
# Usage: scripts/build_windows.sh /path/to/your/SyphonFilterDarkMirror.iso
# Toolchain (TOOLCHAIN=...):
#   llvm-mingw (default if x86_64-w64-mingw32-clang is on PATH): Clang + LLD + mingw-w64 UCRT +
#     libc++ (cmake/llvm-mingw.cmake); SDL2 for Windows in SDL2_WIN (default /opt/sdl2-win).
#   gcc: x86_64-w64-mingw32-g++ (win32 threads, cmake/mingw-w64.cmake) and
#     third_party/SDL2-2.30.9 (scripts/fetch_deps.sh).
# The package contains code recompiled from your own game image: it is for your own use only and
# must not be distributed (dist/ is not committed).
set -euo pipefail
iso=$(realpath "${1:?usage: $0 GAME.iso}")
cd "$(dirname "$0")/.."
tc=${TOOLCHAIN:-$(command -v x86_64-w64-mingw32-clang >/dev/null && echo llvm-mingw || echo gcc)}
if [ "$tc" = llvm-mingw ]; then
  sdl=${SDL2_WIN:-/opt/sdl2-win}
  cmake -S . -B build/win64 -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/llvm-mingw.cmake -DCMAKE_BUILD_TYPE=Release \
        -DSDL2_DIR="$sdl/lib/cmake/SDL2" -DSF_GAME_ISO="$iso"
  dll="$sdl/bin/SDL2.dll"
else
  cmake -S . -B build/win64 -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake -DCMAKE_BUILD_TYPE=Release -DSF_GAME_ISO="$iso"
  dll=third_party/SDL2-2.30.9/x86_64-w64-mingw32/bin/SDL2.dll
fi
cmake --build build/win64 -j"$(nproc)" --target SyphonFilterDarkMirror ca_psprun
out=dist/SyphonFilterDarkMirror-win64
rm -rf "$out" && mkdir -p "$out/game"
cp build/win64/SyphonFilterDarkMirror.exe build/win64/ca_psprun.exe "$dll" "$out/"
cp README.md PROGRESS.md LICENSE "$out/"
cp docs/LEEME_WINDOWS.txt "$out/LEEME.txt"
echo "Copia aqui tu ISO de Syphon Filter: Dark Mirror con el nombre SyphonFilterDarkMirror.iso" > "$out/game/PON_AQUI_TU_ISO.txt"
(cd dist && rm -f SyphonFilterDarkMirror-win64.zip && zip -qr SyphonFilterDarkMirror-win64.zip SyphonFilterDarkMirror-win64)
echo "Package: dist/SyphonFilterDarkMirror-win64.zip"
