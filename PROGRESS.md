# PROGRESS — Syphon Filter: Dark Mirror PC

Procedencia: **[CA]** reutilizado del motor de `AndyLatimer/CodedArmsPC` y
`AndyLatimer/codedArmsContagionPC` (commit 25197d8) · **[PPSSPP]** código de PPSSPP (GPL-2.0+) ·
**[NUEVO]** escrito para este proyecto.

## Estado actual
Hitos 1, 2 y 4 completados; Hito 3 (runtime) avanzado. El juego recompilado arranca sin emulador y
llega, con el renderizador GE→OpenGL, al **primer nivel en 3D** (Episodio 1, parte 1, refinería
KemSynth, con su cinemática del motor) pasando por menús, perfil, guardado, dificultad, vídeo de
introducción y briefing (2026-10-31). Falta jugar: controlar a Gabe (Hito 6).
Último hito completado: 4 (renderizado de contenido auténtico). Disco: UCES-00310 (Europa).

## Verificado (2026-10-30, Linux, GCC 13, SDL2 2.30.9)
- [x] [CA] Motor completo integrado: recompilador Allegrex→C, runtime HLE, GPU GE→OpenGL, SAS,
  Atrac, MPEG/PSMF, sceUtility (guardado), front end SDL2, `ca_psprun`. El runtime (objeto
  `ca_psp`) compila entero sin el juego.
- [x] [NUEVO] Generalización por DISC_ID: `recompile.py` lee PARAM.SFO (o `--game-id`), escribe
  `game_id.txt`; CMake elige `src/games/<ID>/game_hooks.cpp` o `src/games/generic/` (sin ganchos);
  estado compartido en `src/games/common/host_state.cpp`. Lo de Contagion, archivado en `docs/cac/`.
- [x] [NUEVO/PPSSPP] Módulo principal cifrado: `src/psp/boot_module.cpp` usa BOOT.BIN si está en
  claro o descifra EBOOT.BIN con `pspDecryptPRX` (PPSSPP `PrxDecrypter.cpp`, recortado) + libkirk;
  lo usan el runtime (`kernel.cpp`), la herramienta `sf_decrypt` y la configuración de CMake.
- [x] [NUEVO] El recompilador rechaza un BOOT.BIN vacío o cifrado en vez de traducir basura.
- [x] Nombres, rutas y carpeta de preferencias propias (`SyphonFilterDarkMirrorPC`): no se mezclan
  partidas ni ajustes con Contagion.
- [x] Pruebas sin juego: `ctest` 4/4 (`inspect_iso`, `savedata`, `boot_module`, `recomp_tools`).
- [x] Configuración con ISO sintética (BOOT.BIN vacío + EBOOT.BIN en claro): ID detectado,
  `sf_decrypt` del host compilado y ejecutado, módulo entregado al recompilador.

## Verificado con el juego (UCES-00310, Linux, `ca_psprun --gl` en Xvfb)
- [x] Análisis del disco (`REVERSE_ENGINEERING.md`): BOOT.BIN en claro, 267 importaciones con nombre.
- [x] Recompilación: 21 765 funciones, 0 instrucciones sin soporte (datos en `.text` declarados en
  `tools/recomp/games/UCES00310/data.tsv`). Compila y enlaza `SyphonFilterDarkMirror` y `ca_psprun`.
- [x] Arranque: sonido SCREAM/SAS, hilos de streaming, carga de HGZ/HOG, música ATRAC3+.
- [x] Memory Stick: el juego usa comandos FAT (`fatms0:` 0x02415821/0x02425823), añadidos; sin ellos
  mostraba "Memory Stick not found".
- [x] Secuencia observada con capturas: aviso de autoguardado → título animado ("PRESS THE START
  BUTTON") → crear perfil con el teclado propio del juego → **guardado en `ms0:/PSP/SAVEDATA/UCES003100000`**
  ("Save completed") → al volver a arrancar, el perfil se carga → **menú principal** (Single Player,
  Multiplayer, Options, Profiles, Game Sharing) → **menú de Un jugador** (Training, Story, Mission,
  Career Rating, Locked Files).
- [x] HLE añadido (PPSSPP): tiempo de sistema, PollEventFlag, ReferSemaStatus, SleepThreadCB,
  dispatch, GetThreadmanIdList, MaxFreeMemSize, cachés, GPO, ReadBufferPositive, latch del mando,
  DmacMemcpy, UMD, energía, AtracGetNextSample, IoChangeAsyncPriority. Ninguna llamada sin implementar
  hasta el menú de Un jugador.
- [x] El generador solo reescribe el C que cambia (salida determinista, comprobada byte a byte): una
  reconfiguración ya no recompila 100 MB de C.

- [x] **Story Mode → Normal → vídeo de introducción (PSMF/H.264, decodificado) → briefing
  "Episode 1, Part 1: Insertion Point Alpha" → nivel REFIN_A1 cargado y renderizado en 3D**
  (escenario nevado, iluminación, partículas, personaje animado en cinemática).
- [x] Estado de espera de los hilos (`waitType`/`waitId` en `sceKernelReferThreadStatus`): el
  streaming de sonido espera a que sus hilos bloqueen en sus semáforos antes de un vídeo.
- [x] Vídeos con cabecera PSMF incompleta (el juego solo pasa 0x54 bytes): `MediaEngine` declara el
  stream H.264 0xE0 (el FFmpeg mínimo no puede sondearlo como hace PPSSPP).

## Parcialmente implementado / sin verificar
- [ ] Descifrado real de un EBOOT.BIN `~PSP`: compila y rechaza contenedores inválidos; no hay
  vector de prueba real hasta tener un disco que lo necesite (puede que Dark Mirror no lo necesite).
- [ ] Módulos descifrados comprimidos (gzip): detectados y rechazados con mensaje; sin inflado.
- [ ] Compilación de Windows: no hay MinGW en este entorno (sin acceso a apt/pip/npm ni a las
  descargas de GitHub); `scripts/build_windows.sh` es el de Contagion, adaptado, sin ejecutar aquí.
- [ ] Controles de PC: el front end es el de Contagion (FPS). Sin ganchos, el ratón no mueve la
  cámara; teclado y mando se traducen a botones/stick de PSP.

## Problemas conocidos
- `sqrt.s` con NaN negativo (heredado, 3 palabras en `cpu_diff`).
- Ajustes de GPU de Contagion activados por contenido (`REVERSE_ENGINEERING.md` §3): revisar.

## Decisiones arquitectónicas
Ver `ARCHITECTURE.md`. Resumen: recompilación estática + HLE (alternativa C); SDL2 por ahora
(DualSense USB completo vía HIDAPI), SDL3 en el roadmap; OpenGL 3.3; nada del juego en git.

## Archivos modificados respecto a Contagion
Nuevos: `src/psp/boot_module.*`, `src/psp/ppsspp/PrxDecrypter.*`, `PrxDecrypterCompat.h`,
`src/games/{common,generic}/`, `tools/decrypt/`, `tools/recomp/disc_id.py`,
`tests/test_recomp_tools.py`, `tests/boot_module/`, documentos de la raíz.
Cambiados: `CMakeLists.txt`, `tools/recomp/{recompile,codegen}.py`, `src/psp/kernel.cpp` (carga
del módulo), cadenas y rutas de `src/frontend/`, `src/core/Config.cpp`, `src/psp/psprun.cpp`,
`scripts/`. Movidos a `docs/cac/`: notas, progreso, ganchos y tablas de Contagion.

## Siguiente tarea concreta
1. Primera misión: pasar la cinemática del motor y mover a Gabe (stick, cámara, disparo) con
   `ca_psprun --control`; comprobar audio (`--wav`).
2. `cpu_diff` completo (tarda >15 min con 2 núcleos: lanzarlo en segundo plano).
3. Probar el ejecutable con ventana (`SyphonFilterDarkMirror`) y la compilación de Windows.

Secuencia reproducible hasta el primer nivel (con un perfil ya guardado en `build/run/memstick`):
`ca_psprun --iso ... --gl --seconds 125 --press CROSS@21 --press START@24 --press CROSS@28
 --press DOWN@31 --press CROSS@32.5 --press CROSS@46 --press START@55 --press CROSS@57 --press CROSS@75
 --shot 90:nivel.png` (aviso → título → Un jugador → Story → Normal → saltar vídeo → briefing → nivel).
