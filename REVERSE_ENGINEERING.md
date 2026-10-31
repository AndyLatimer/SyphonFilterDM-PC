# REVERSE_ENGINEERING — Syphon Filter: Dark Mirror (PSP)

Regla: aquí solo se escribe lo **verificado** con la copia del usuario, citando la herramienta
que lo muestra. Lo que procede de fuentes externas se marca **(sin verificar)**.

## 1. Imagen de referencia
`SyphonFilterDarkMirror.iso` — **UCES-00310** (Europa), 1 775 894 528 bytes.
MD5 `d8715783c35dddf46f323277d61799fb`, SHA-1 `11c3793e440ee2f47a3fb410530d8b3376a392f2`, CRC32 `642627bb`.
UMD_DATA.BIN `UCES-00310|FE0C11681D3226AC|0001|G`. PARAM.SFO: TITLE `Syphon Filter™ Dark Mirror`,
DISC_VERSION 1.00, PSP_SYSTEM_VER 2.60, PARENTAL_LEVEL 7. 1 261 ficheros.
Fuente: `tools/inspect/inspect_iso.py` (informe en `analysis/inspect.json`, no se sube).

## 2. Ejecutables
- `SYSDIR/BOOT.BIN` 8 877 968 B, SHA-1 `1a9f5e7393bb61c7671785f3a3cbd2f6ad5d3b1f`: **PRX ELF en claro**
  (tipo 0xFFA0), módulo `LogansShadow`, 64 secciones, entrada 0x08ad11c4 (base 0x08804000).
  `.text` 0x08804000–0x08dba590 (5,7 MB). Reubicaciones de tipo 2/4/5/6 únicamente
  (49 523 / 80 796 / 54 009 / 77 331): el cargador PRX del motor vale sin cambios.
  No hace falta descifrar EBOOT.BIN (`~PSP`, mismo módulo).
- 267 funciones importadas de 31 bibliotecas, **todas con nombre** en las tablas HLE de PPSSPP
  (`tools/recomp/games/UCES00310/imports.tsv`, de `gen_imports.py`).
- `USRDIR/IRX/`: 15 PRX + `IOPRP300.IMG`, todos de Sony: cifrados `~SCE`/`~PSP` (audiocodec,
  atrac3plus, mpeg, mpegbase, sascore, videocodec, ifhandle, memab, adhoc_auth, ap_dialog_dummy)
  y en claro los de red (`PSPNET*.PRX`). Ninguno es código del juego: el runtime los sustituye (HLE)
  y la red no se implementa. A diferencia de Contagion, no hay módulos del juego aparte.
- `SYSDIR/UPDATE/`: actualización de firmware (no se usa).

## 3. Motor (cadenas de BOOT.BIN)
- Rutas de compilación conservadas en mensajes de aserción: `c:/Uberbuild_DMPSP/Dark_Mirror_PSP/Code/...`
  - `Code/Game/Source/*.cc` (lógica: `actor.cc`, `ai.cc`, `cinemaCamera.cc`, `Searchlight.cpp`…)
  - `Code/Libs/CosmosLib` (biblioteca base del motor: contenedores, UI `UiFrame`/`UiFrameMan`)
  - `Code/Libs/989snd/Skat` (sonido de SCEA "989 Sound": Skat)
  - `Code/Libs/Rtime/.../scertpp` (red SCE-RT/Medius: sesiones, clanes, ladder) — no se implementará.
- Nombres de clases C++ (RTTI) presentes: `cGameCamera`, `cCinemaCamera`, `cPopCamera`,
  `cGameGobjController`, `cAIMessage`… Útiles para localizar cámara y control del jugador (Hito 5).
- Control y cámara (cadenas, sin analizar aún el código): clases `cPlayerCamera`, `cPlayerController`
  (`PlayerController.cc`), `cPlayerControls`, `cInputMap`/`cBaseInputMap` (carga
  `COMMON\inputmap.dat` con `LoadInputMap`), `cInputSys`; opción `OPT_AIM_INVERT`. Cientos de
  parámetros de cámara con nombre por modo: `CAMERA_Mode_<Climb|Crawl|BuddyClimb|...>_CamArm_
  {Extension,PivotOffset}_{Forw,Rght,Up}` y `_Rotation_{Horz,Vert}` — candidatos para una cámara
  al hombro sin tocar la lógica (cambiando los valores del propio juego).

## 4. Formatos de datos (cabeceras observadas)
| Ext. | N.º | MB | Formato observado |
|---|---|---|---|
| `.HOG` | 296 | 457 | archivo contenedor: magia `iiii`, versión 1, tabla de entradas con nombre (p. ej. `SHELL.BNK`, bloques `SBlk` de sonido) |
| `.HGZ` | 581 | 319 | `HGZ\0` + tamaños + flujo **zlib** (`78 9c`): HOG comprimido |
| `.SKX` | 37 | 218 | `SKEX`: audio en streaming de Skat (música/voces) |
| `.PMF` | 39 | 512 | vídeo PSMF (`PSMF0014`): cinemáticas |
| `.AT3` | 186 | 134 | ATRAC3 en RIFF/WAVE: música de menús y bonus, voces de clasificación |
| `.TDX` | 39 | 0,5 | texturas sueltas |
| `.SUB` | 24 | 0,2 | subtítulos de vídeo en texto (`Font "DEFAULT.FNT"`, `NumLines`) |
| `STRINGS.DAT` | | 0,1 | textos y diálogos con identificadores (`31_10_10_1`) |
| `INPUTMAP.DAT` | | 0,002 | parámetros de entrada en `double` (curvas de stick/apuntado) — revisar en el Hito 5 |

Estructura: `GAMEDATA/SINGLE/<MISIÓN>_1P/<SUBNIVEL>/` (822 MB, campaña), `MULTI/` (156 MB),
`FRONTEND/` (menús, bonus, voces por idioma), `COMMON/`, `GAMESHARE/`, vídeos en `USRDIR/`.

## 5. Recompilación de BOOT.BIN  ✅
- 21 765 funciones, **ninguna instrucción sin soporte** (`recompile.py`).
- Datos dentro de `.text`: 0x08ba0080–0x08ba58e0 (22 KB, parecen texturas de 4 bits). Su dirección
  se toma en el código, así que el descubrimiento los tomaba por 6 funciones con 932 palabras
  indescodificables. Se declaran en `tools/recomp/games/UCES00310/data.tsv`; el recompilador
  informa de cualquier función con palabras desconocidas para detectar casos así.

## 6. Lo heredado que hay que revisar con este juego
`src/psp/gpu/button_glyphs.cpp` (repinta símbolos de botones al reconocer texturas de Contagion),
`minimap.cpp` y `ads.cpp` (inactivos sin ganchos). Comprobar que no se disparan en Dark Mirror.

## 7. Cámara del jugador (en curso)
- Volcados de memoria girando con ○/□/△ (`ca_psprun` `memdump`, `scripts/dev/cmds/cam_scan2.txt`):
  en `.bss` 0x08e9c870–0x08e9c8a8 hay una matriz 2×2 de giro y dos ángulos en grados que cambian
  con ○/□ pero no con △: parece el radar, no la cámara. Pendiente con la compilación de vigilancia.
