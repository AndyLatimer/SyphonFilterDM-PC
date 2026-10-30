# Syphon Filter: Dark Mirror — port nativo para PC

Objetivo: `SyphonFilterDarkMirror.exe` (Windows x64) que ejecuta el código original del juego de PSP
(SCE Bend Studio, 2006) **recompilado estáticamente** a código nativo, con un runtime que sustituye
al sistema de la PSP, un renderizador GE→OpenGL, controles de acción en tercera persona para PC y
soporte de DualSense por USB. No es un emulador: no hay intérprete ni JIT de MIPS en el ejecutable.

Basado en el motor de los ports de [Coded Arms](https://github.com/AndyLatimer/CodedArmsPC) y
[Coded Arms: Contagion](https://github.com/AndyLatimer/codedArmsContagionPC) (recompilador
Allegrex→C, runtime HLE, GPU, audio, vídeo, guardado y front end ya probados con dos juegos).
El repositorio **no contiene ningún recurso del juego**: todo se lee de la copia del usuario
(`game/`, ignorada por git) y el código generado se queda en `build/`.

**Estado:** motor integrado y generalizado; a la espera de la ISO del usuario para el Hito 1.
Detalles en [`PROGRESS.md`](PROGRESS.md).

| Documento | Contenido |
|---|---|
| [`PROGRESS.md`](PROGRESS.md) | estado, verificado, pendiente, siguiente tarea |
| [`ARCHITECTURE.md`](ARCHITECTURE.md) | alternativas evaluadas y arquitectura elegida |
| [`REVERSE_ENGINEERING.md`](REVERSE_ENGINEERING.md) | lo verificado del juego (formatos, módulos, motor) |
| [`BUILD.md`](BUILD.md) | compilar en Linux y Windows |
| [`TESTING.md`](TESTING.md) | pruebas y cómo reproducirlas |
| [`ROADMAP.md`](ROADMAP.md) | hitos 1–9 |

Compilar (resumen; ver `BUILD.md`):
```
cmake -S . -B build/linux -G Ninja -DSF_GAME_ISO=$PWD/game/SyphonFilterDarkMirror.iso
cmake --build build/linux
ctest --test-dir build/linux
```

```
src/psp/                 runtime HLE (sistema de la PSP), GPU GE→OpenGL, carga del módulo   [CA]
src/frontend/            ventana, entrada, audio, menús del PC                              [CA]
src/games/common/        estado compartido front end ↔ ganchos del juego
src/games/generic/       sin ganchos (el juego tal cual, controles de PSP)
src/games/<DISC_ID>/     ganchos de Dark Mirror (cuando existan)
tools/recomp/            recompilador estático Allegrex → C                                  [CA]
tools/recomp/games/<ID>/ direcciones de los ganchos y módulos extra de cada disco
tools/decrypt/           sf_decrypt: módulo principal en claro (BOOT.BIN o EBOOT.BIN descifrado)
tools/inspect/           inspect_iso.py: análisis del Hito 1 sobre la ISO/CSO
docs/cac/, docs/ca1/     lo específico de Contagion y Coded Arms, como referencia
game/                    tu copia del juego (no se sube)
```

Licencia: GPL-2.0-or-later (incluye código de PPSSPP); los binarios que enlazan libkirk son GPL-3.0.
