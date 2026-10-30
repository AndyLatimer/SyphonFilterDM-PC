# PROGRESS — Syphon Filter: Dark Mirror PC

Procedencia: **[CA]** reutilizado del motor de `AndyLatimer/CodedArmsPC` y
`AndyLatimer/codedArmsContagionPC` (commit 25197d8) · **[PPSSPP]** código de PPSSPP (GPL-2.0+) ·
**[NUEVO]** escrito para este proyecto.

## Estado actual
Hito 0 (motor) completado. **Bloqueado para el Hito 1: falta la ISO del usuario** en `game/`.
Último hito completado: ninguno de los hitos 1–9 (ver `ROADMAP.md`).

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
1. Copiar la ISO a `game/SyphonFilterDarkMirror.iso` y ejecutar
   `python3 tools/inspect/inspect_iso.py game/SyphonFilterDarkMirror.iso --json analysis/inspect.json`.
2. Rellenar `REVERSE_ENGINEERING.md` §1–2 con lo verificado; generar
   `tools/recomp/games/<ID>/imports.tsv` con `gen_imports.py`.
3. Recompilar (`cmake -DSF_GAME_ISO=...`), pasar `cpu_diff` y arrancar en `ca_psprun --trace`
   hasta la primera función HLE que falte.
