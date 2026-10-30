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
Compilación cruzada desde Linux/WSL con MinGW-w64 (modelo de hilos win32, GCC 13+):
```
sudo apt install g++-mingw-w64-x86-64 ninja-build cmake zip
scripts/fetch_deps.sh                      # SDL2 2.30.9 para MinGW
scripts/build_windows.sh game/SyphonFilterDarkMirror.iso
```
Resultado: `dist/SyphonFilterDarkMirror-win64.zip` (exe, `SDL2.dll`, documentación).
El ejecutable es estático salvo `SDL2.dll`; no requiere PPSSPP ni otras dependencias.
