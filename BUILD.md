# BUILD

El juego **no** está en el repositorio. La compilación lee tu ISO, recompila su código a C en
`build/<plataforma>/generated/` y lo enlaza con el runtime. Ese código derivado y los ejecutables
resultantes son para tu uso personal: no se suben ni se distribuyen.

## Linux (desarrollo y pruebas)
Requisitos: CMake ≥ 3.16, Ninja, GCC/G++ 13+, Python 3, SDL2 2.30 (paquete `libsdl2-dev`, o
compilado desde https://github.com/libsdl-org/SDL `release-2.30.9` e indicado con `-DSDL2_DIR`).
FFmpeg estático ya viene en `third_party/ffmpeg/`.

```
# solo runtime, herramientas y pruebas (sin el juego)
cmake -S . -B build/linux -G Ninja
cmake --build build/linux && ctest --test-dir build/linux

# con el juego
cmake -S . -B build/linux -G Ninja -DSF_GAME_ISO=$PWD/game/SyphonFilterDarkMirror.iso
cmake --build build/linux
./build/linux/SyphonFilterDarkMirror            # o ca_psprun para pruebas sin ventana
```
Opciones: `-DSF_GAME_ELF=BOOT.bin` (módulo ya descifrado), `-DCA_RECOMP_WATCH=ON` (vigilancia de
escrituras), `-DCA_RECOMP_PROFILE=ON`, `-DCA_RECOMP_TRACE=dir1,dir2`.

Si el BOOT.BIN del disco está vacío o cifrado, la configuración compila `tools/decrypt` con el
compilador del sistema, extrae el módulo de EBOOT.BIN y recompila ese (`build/.../BOOT.decrypted.bin`).

## Windows x64 (`SyphonFilterDarkMirror.exe`)
`scripts/build_windows.sh game/SyphonFilterDarkMirror.iso` → `dist/SyphonFilterDarkMirror-win64.zip`
(exe estático salvo `SDL2.dll`; usa la UCRT de Windows 10/11; no requiere PPSSPP).

Dos cadenas de compilación cruzada, desde Linux/WSL:
- **llvm-mingw** (Clang + LLD + mingw-w64 UCRT + libc++; `cmake/llvm-mingw.cmake`): la publicada en
  https://github.com/mstorsjo/llvm-mingw (bin/ en PATH), o montada sobre un Clang del sistema con sus
  scripts. Así se hizo en el entorno de desarrollo (Clang 18.1.3, sin red para paquetes):
  ```
  git clone https://github.com/mstorsjo/llvm-mingw && cd llvm-mingw
  git clone https://github.com/mingw-w64/mingw-w64
  git clone --filter=blob:none --no-checkout --depth 1 -b llvmorg-18.1.3 https://github.com/llvm/llvm-project
  (cd llvm-project && git sparse-checkout set compiler-rt libcxx libcxxabi libunwind runtimes cmake llvm/cmake third-party && git checkout)
  # libunwind 18 con las cabeceras actuales de mingw-w64: en libunwind/src/Unwind-seh.cpp,
  # "return (EXCEPTION_DISPOSITION)4", "switch ((int)ms_act)" (cambio posterior de LLVM)
  export PREFIX=/opt/llvm-mingw TOOLCHAIN_ARCHS=x86_64 PATH=/opt/llvm-mingw/bin:$PATH
  ./install-wrappers.sh $PREFIX --host-clang=clang
  ./build-mingw-w64.sh $PREFIX --with-default-msvcrt=ucrt --disable-cfguard
  ./build-compiler-rt.sh $PREFIX --disable-cfguard
  ./build-libcxx.sh $PREFIX --disable-cfguard
  ./build-mingw-w64-libraries.sh $PREFIX --disable-cfguard
  ```
  SDL2 para Windows (HIDAPI para el DualSense, XInput, OpenGL), con la misma cadena:
  `cmake -S SDL -B sdl-win -G Ninja -DCMAKE_TOOLCHAIN_FILE=<repo>/cmake/llvm-mingw.cmake -DCMAKE_INSTALL_PREFIX=/opt/sdl2-win -DSDL_TEST=OFF && cmake --build sdl-win && cmake --install sdl-win`
- **GCC MinGW-w64** (modelo de hilos win32, GCC 13+; `cmake/mingw-w64.cmake`):
  `sudo apt install g++-mingw-w64-x86-64 ninja-build cmake zip`, `scripts/fetch_deps.sh`,
  `TOOLCHAIN=gcc scripts/build_windows.sh ...`
