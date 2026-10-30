# ROADMAP

Ningún hito se marca como hecho sin evidencia reproducible (`TESTING.md`).

| Hito | Entregable | Base reutilizada de CA/CAC | Estado |
|---|---|---|---|
| 0. Motor | runtime, recompilador, GPU, audio, vídeo, guardado, front end generalizados por DISC_ID | todo | ✅ compila; pruebas sin juego 4/4 |
| 1. Análisis | informe del disco en `REVERSE_ENGINEERING.md` | `inspect_iso.py`, `gen_imports.py` | ⏳ falta la ISO |
| 2. Recompilación | BOOT.BIN recompilado sin instrucciones sin soporte; `cpu_diff` | recompilador + `cpu_diff` | ⏳ |
| 3. Runtime | arranque en `ca_psprun` hasta el primer fotograma; HLE que falte | kernel HLE (~270 funciones) | ⏳ |
| 4. Renderizado | menú y escena auténticos en ventana | GE→OpenGL 3.3 | ⏳ |
| 5. Controles | teclado/ratón (cámara directa por gancho), DualSense USB, remapeo | `Input.cpp`, ratón directo de CAC | ⏳ |
| 6. Jugabilidad | primera misión con objetivos, cámara, sonido | — | ⏳ |
| 7. Campaña | todas las misiones, progresión documentada | — | ⏳ |
| 8. Gráficos | resolución interna, 16:9/16:10/21:9 sin estirar, FOV, AA, anisotrópico | escalado y panorámico de CA/CAC | ⏳ |
| 9. Versión PC | menú de ajustes, .ini, guardado, paquete Windows | `OptionsMenu`, `Config`, `build_windows.sh` | ⏳ |

Tareas transversales:
- Cámara de hombro en tercera persona con ratón: gancho en el integrador de cámara del juego
  (como el giro del jugador en CAC), conservando las cámaras de las cinemáticas.
- Esquema de controles definitivo según las acciones reales del juego (el del prompt es provisional).
- Iconos PlayStation/teclado según el dispositivo (`button_glyphs`, ya hecho para CAC).
- DualSense: vibración (las llamadas del juego a la vibración no existen en PSP: habrá que
  derivarlas de eventos), giroscopio opcional, gatillos adaptativos opcionales.
- Migración a SDL3 (aislada en `src/platform` y `src/frontend`).
- FPS > 30/60: solo tras medir cómo depende la lógica del tiempo.
- `sqrt.s` con NaN negativo (3 palabras en `cpu_diff`, heredado).
- CSO: solo `inspect_iso` lo lee; el runtime y el recompilador necesitan ISO (convertir con PPSSPP o maxcso).
