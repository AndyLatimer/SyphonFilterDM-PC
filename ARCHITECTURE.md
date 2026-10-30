# ARCHITECTURE — Syphon Filter: Dark Mirror PC

## 1. Alternativas evaluadas

| | Viabilidad | Fidelidad | Coste | Veredicto |
|---|---|---|---|---|
| **A. Decompilación a C legible** | Baja a corto plazo: un juego de ~5–6 MB de código MIPS sin símbolos son años de trabajo manual (los proyectos de decompilación "matching" de N64/PS2 tardan años con decenas de personas). | Máxima al terminar; nula mientras tanto (no se puede jugar a medias). | Muy alto | Descartada como base; se usará **por funciones** para lo que haya que modificar (cámara, controles). |
| **B. Recompilación estática pura** | Alta para el código MIPS; insuficiente sola: el juego llama al firmware (hilos, ficheros, GE, audio, vídeo, guardado). | Máxima: es el código original. | Medio | Necesita C. |
| **C. Recompilación estática + capa de compatibilidad PSP (HLE)** | **Demostrada** en este mismo motor con dos juegos (Coded Arms y Contagion: menús, vídeos, música, guardado, misiones jugables). | Máxima: código original del juego; solo el sistema operativo se sustituye. | Medio-bajo gracias a la reutilización | **Elegida.** |
| **D. Reconstrucción progresiva de subsistemas** | Baja: reescribir lógica, IA y misiones es recrear el juego (contrario a las reglas del proyecto). | Baja/variable | Muy alto | Descartada; solo para mejoras aisladas (renderizado, entrada). |

Proyectos externos revisados: PS2Recomp, psxrecomp, N64Recomp (misma idea en otras consolas),
y ports PSP recompilados publicados en 2026 (p. ej. *Vice City Stories*, *Ace Combat X*,
*Phantasy Star Portable 2 Infinity*). Todos confirman el enfoque C. Nuestro recompilador ya cubre
Allegrex con VFPU y está validado contra el intérprete de PPSSPP (`cpu_diff`), así que no
aporta adoptar otro.

## 2. Arquitectura elegida (C)

```
 ISO del usuario ─► tools/decrypt (si BOOT.BIN no está en claro)
        │                    │
        ▼                    ▼
 tools/recomp/recompile.py  ── BOOT.BIN (+ PRX del juego) ──► build/generated/*.c  (no se sube)
        │
        ▼
 SyphonFilterDarkMirror.exe
   ├─ código del juego recompilado (una función C por función MIPS; mismo control de flujo)
   ├─ src/psp/      runtime: memoria de 32 MB, hilos con apropiación, kernel HLE, E/S de UMD
   │                y memory stick, GE→OpenGL 3.3, SAS/Atrac/MPEG (PPSSPP), sceUtility (guardado)
   ├─ src/games/<ID>  ganchos del port (cámara/ratón/estado), en direcciones verificadas
   └─ src/frontend/ SDL2: ventana, teclado/ratón, mandos, audio, menú de ajustes, .ini
```

Principios (heredados de CA/CAC y confirmados allí):
- El código del juego no se reescribe. Las mejoras entran por **ganchos** en direcciones concretas
  (`tools/recomp/games/<ID>/hooks.tsv`) que leen/escriben el estado del juego, o en el
  **renderizador** (resolución, panorámico, filtros) sin tocar la lógica.
- Lo específico de cada disco se separa por DISC_ID; el motor es común.
- Los FPS: se mantiene la frecuencia lógica del juego hasta verificar cómo depende del tiempo.

## 3. Decisiones

- **SDL2 2.30 en vez de SDL3 (por ahora).** El front end ya usa SDL2, cuyo controlador HIDAPI
  soporta el DualSense por USB sin Steam: botones, vibración, LED, giroscopio
  (`SDL_GameControllerSetSensorEnabled`) y efectos de gatillos (`SDL_GameControllerSendEffect`).
  Migrar a SDL3 no desbloquea nada para el DualSense y arriesga lo probado; queda en `ROADMAP.md`
  como tarea aislada (la API está encapsulada en `src/platform/` y `src/frontend/`).
- **OpenGL 3.3** como en CA/CAC (probado en Windows). Vulkan/D3D no aportan a un juego de PSP;
  el renderizador está aislado si hiciera falta.
- **Descifrado del ejecutable** con `pspDecryptPRX` de PPSSPP + libkirk, en el runtime y en la
  compilación, por si el BOOT.BIN del disco está vacío o cifrado (`src/psp/boot_module.cpp`).
- **Nombres internos `ca_*`** (bibliotecas CMake, `ca_psprun`, variables `CA_*`): son los del
  motor común; se mantienen para poder llevar correcciones entre los tres proyectos.
