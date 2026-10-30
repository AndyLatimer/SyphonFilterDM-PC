# REVERSE_ENGINEERING — Syphon Filter: Dark Mirror (PSP)

Regla: aquí solo se escribe lo **verificado** con la copia del usuario, citando la herramienta
que lo muestra. Lo que procede de fuentes externas o de memoria se marca **(sin verificar)**.

## 0. Pendiente de la ISO del usuario

Todavía no hay imagen del juego en el proyecto. Nada de lo siguiente está verificado.

## 1. Datos externos (sin verificar)

- Desarrollador: SCE Bend Studio; PSP, 2006. Usó un motor propio (no relacionado con los de
  Coded Arms/Contagion).
- Identificadores de disco conocidos: UCUS-98641 (EE. UU.), UCES-00141 (Europa). Los ganchos
  irán en `src/games/<ID sin guion>/` y `tools/recomp/games/<ID sin guion>/` según el disco.
- Tiene multijugador ad hoc e infraestructura (fuera del alcance; la red no se implementa).

## 2. Qué se medirá en el Hito 1 (herramientas listas)

| Pregunta | Herramienta |
|---|---|
| ID, versión, firmware, hashes | `tools/inspect/inspect_iso.py GAME.iso` |
| BOOT.BIN en claro / vacío / cifrado | `inspect_iso.py`, `tools/recomp/disc_id.py --boot-state` |
| Módulos PRX del juego y de Sony (cifrados → HLE) | `inspect_iso.py` (lista `USRDIR`/`SYSDIR`) |
| Importaciones y cuáles faltan en el runtime | `tools/recomp/gen_imports.py`, `CA_HLE_STATS`, `--trace` |
| Funciones recompiladas, instrucciones sin soporte | `tools/recomp/recompile.py` |
| Corrección de la CPU recompilada | `cpu_diff` (intérprete de referencia de PPSSPP) |
| Arranque, ficheros que abre, hilos | `ca_psprun --seconds N --trace`, `CA_THREAD_DUMP` |
| Formatos de archivo (contenedores, compresión) | volcado de lecturas `sceIo*` + inspección |
| Direcciones de cámara/jugador para los ganchos | build de vigilancia `-DCA_RECOMP_WATCH=ON` + `CA_WATCH_ADDR` |

## 3. Lo heredado que hay que revisar con este juego

El runtime contiene ajustes hechos para Contagion que se activan por contenido, no por disco:
`src/psp/gpu/button_glyphs.cpp` (repinta símbolos de botones al reconocer sus texturas),
`src/psp/gpu/minimap.cpp` (minimapa, desactivado sin ganchos), `src/psp/gpu/ads.cpp`
(solo con ADS activo). Comprobar que no se disparan en Dark Mirror.
