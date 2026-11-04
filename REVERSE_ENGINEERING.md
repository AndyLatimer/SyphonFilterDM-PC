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
Método: volcados de RAM de usuario (`ca_psprun` `memdump`) con ○/□/△ mantenidos
(`scripts/dev/cmds/cam_scan3.txt`), compilación de vigilancia (`build/watch`, `CA_WATCH_ADDR`,
`CA_WATCH_STACK`), `addf ADDR DELTA` en el FIFO de `ca_psprun`, `tools/recomp/disasm.py`.
- **La cámara tiene inercia**: tras soltar ○ los ángulos siguen girando unos 0,3 s.
- **Pila de matrices** del motor: objeto en 0x08e8d450, índice en +0x1080, matrices de 64 bytes en
  +0xC80; push en 0x08b86140 (copia la cima) y multiplicación en 0x08b86098. 0x08e8e280 (y su
  copia 0x09fbf140) es la traslación de una de sus matrices: derivado, no el ángulo.
- 0x08e9c870–0x08e9c8a8 (`.bss`): matriz 2×2 de giro y dos ángulos en grados (0x08e9c8a0/a4) que
  giran con ○/□ y no con △; escritos desde 0x08c19af4/0x08c19b30 y 0x08901f30. Probable radar/HUD.
- 0x08ec2660–0x08ec26ff: planos del frustum de la vista (derivados).
- Siguiente paso: vigilar quién escribe la matriz raíz de la pila (la vista de la cámara) y subir
  por `CA_WATCH_STACK` hasta el código de `cPlayerCamera` que la calcula a partir de sus ángulos;
  enganchar allí `g_aim` (como `ca_hook_player_turn` en Contagion) y poner `g_directAim = true`.

### 7.1 Avances (2026-10-31)
- En cobertura (inicio del nivel) el stick solo asoma a Gabe: las pruebas de cámara se hacen tras
  separarlo de la pared.
- **Eje de apuntado**: elemento de una tabla de acciones con temporizador (base 0x099b8ed0, +72
  tiempo, +80 vector): 0x099b8f20 = +1 con ○ / stick a la derecha, −1 con □, 0 en reposo, en los
  dos esquemas. Lo escribe la entrada al cambiar (0x08c4c5fc, 0x08d5ea24) y lo borra el
  temporizador (0x08a7d028). Escribirlo desde fuera no gira la cámara: el consumidor toma el valor
  por otra vía (mensaje).
- **Inercia**: 0x09a37a20 es la velocidad de un cuerpo físico (0x09a379e0) amortiguada en
  0x08d1a7ec (v *= max(0, 1 − k·dt)); integrador en 0x08d1a7c0.
- La orientación de la cámara no aparece como ángulo en memoria (ni en grados ni en radianes):
  probablemente una matriz girada por incrementos. El radar (0x08e9c8a4) la sigue linealmente.
- Esquema Advanced: giro ≈150°/s con el stick a tope; respuesta casi nula por debajo de ~0,5.

## 8. Bucle de fotogramas y límite de 30 fps
- El hilo del bucle principal duerme hasta que la interrupción de vblank lo despierta (manejador
  0x08c193dc: `sceKernelWakeupThread`) y, además, el limitador 0x08c2275c espera otro vblank
  (`sceDisplayWaitVblankStart` desde 0x08c1c9ac) si 1e6 / tiempo del fotograma supera el objetivo
  de objeto+484. Dos vblanks por fotograma: 30 fps. La volteada es 0x08c1ca90 (SetFrameBuf).
- La lógica usa el tiempo medido (0x08c20914): con 60 fps, los relojes del juego avanzan 5,3 s
  en 5,3 s reales (volcados de RAM, `scripts/dev/cmds/gt.txt`).
- Parche (`tools/recomp/games/UCES00310/patches.tsv`): 0x08c227c8 `bc1t` → `b`, el limitador no
  espera nunca. Medido sin renderizador (`CA_NOGL=1`, `CA_VBLANK_TRACE=1`): 59,9 volteadas/s en
  nivel y menús. El tope de 30 del PC (`[video] frame_rate = 30`) se aplica ahora en
  `sceDisplaySetFrameBuf` (dos vblanks entre volteadas): 29,97/s.
- El primer intento (objetivo 60 en 0x08c227a4) no bastaba: la espera del limitador más la del
  despertar seguían dando dos vblanks.

## 9. Candado del entrenamiento 1 (aparcado: el usuario pudo seguir)
- El nivel es `SINGLE/TRAIN_1P/TRAIN_1` (el menú de entrenamiento abre el 1 con X; DERECHA pasa al 2).
- El candado es el objeto `NI_REFB_Latch` de `TRAIN_1/FREED_L.HGZ` (definición en 0x75e0 del flujo
  descomprimido): pieza `NI_REFB_Latch_lattchA.bpm` (textura `KEYHOLELATCH01.TDX`), puerta animada
  `NI_doorA.bpm` y colisión `NI_doorcollA.bpm`; colocado en (71.79, −17.64, 5.44), giro 90.34°.
  En RAM, tras cargar el nivel, hay instancias con esa traslación (p. ej. 0x09890130), así que el
  objeto se crea. El guion del nivel tiene `train_1::EnableHotboxIfLocked`.
- El jugador empieza en (5.6, −53.7, 3.37) (traslación de su matriz en `matdump`).
- Herramientas: `scripts/dev/drive.py` (pruebas en Python que leen la RAM durante la partida),
  `matdump` ahora anota direcciones de vértices y textura de cada dibujo, y F8 en el ejecutable
  guarda la RAM y los dibujos del siguiente fotograma.
- Siguiente paso: con una captura F8 delante de la puerta, ver si hay dibujos con traslación junto
  al candado (si no los hay, el juego no lo manda dibujar: lógica; si los hay, es el renderizador).

## 10. HUD en proyección ortográfica
- El radar y el panel del arma se dibujan en modo transformado con proyección ortográfica
  (escala 2/480 × 2/272, w constante), no en modo "through"; el contador de munición y los textos
  sí van en "through". Volcado `matdump` con `--aspect 2.36`.
- Consecuencias que había en el port: el campo de visión del PC (85° frente a 80°) escalaba
  también el HUD ortográfico y lo separaba del contador de munición; y el panorámico (Hor+)
  enseñaba marcos que el juego deja fuera de x 0..480 (las "flechas" de los bordes). El
  renderizador ya no aplica ninguna de las dos cosas a proyecciones ortográficas.
- Cobertura: el juego solo pega a Gabe si el empujón empieza cuando ya toca la superficie (tras
  soltar el stick). Si se para un poco antes, el empujón lo lleva hasta ella y no cuenta. El botón
  de cubrirse repite empujar 0,6 s / soltar 0,6 s tres veces; probado en el ejecutable de PC
  (`scripts/dev/windowed_test.sh scripts/dev/cmds/win_cover3.xsend`) parando antes de la caja.
- El stick del PC ahora llega a 0 y 255 a fondo (antes 1..255: 128 ± 127).

## 11. Brillos superpuestos y campo de visión; filtrado mejorado
- Los brillos de dianas y marcas se dibujan también con proyección ortográfica, en la posición de
  pantalla que da la proyección del juego. Con un campo de visión del PC distinto de 80° el
  escenario cambia de escala y esos brillos no: quedaba un "aura" alrededor de las dianas desde que
  el HUD ortográfico dejó de escalarse (§10). Campo de visión por defecto: 80 (el del juego); los
  `controls.ini` con 85 se pasan a 80 una vez (`[ADS] FovVersion = 2`).
- Texturas mejoradas (`[video] enhanced_textures`, por defecto 1): a las texturas de un solo nivel
  se les generan mipmaps al subirlas y los dibujos 3D con filtro lineal (sin prueba alfa, para no
  borrar rejas y vallas) usan trilineal + anisotrópico 16x. Los 2D siguen en el nivel 0.
