# TESTING

## Sin el juego (CI; `ctest --test-dir build/linux`)
| Prueba | Qué comprueba |
|---|---|
| `inspect_iso` | análisis de ISO/CSO sintéticas: PARAM.SFO, estado del ejecutable, PRX, importaciones |
| `savedata` | cifrado y hashes del guardado (sceChnnlsv/KIRK); con `-DCA_PPSSPP_DIR` compara con PPSSPP |
| `boot_module` | clasificación del ejecutable (claro/vacío/cifrado) y rechazo limpio de contenedores inválidos |
| `recomp_tools` | DISC_ID y estado de BOOT.BIN en imágenes sintéticas; `sf_decrypt` extrae EBOOT.BIN en claro tras un BOOT.BIN vacío; el recompilador rechaza un BOOT.BIN vacío |

## Con el juego
| Prueba | Cómo |
|---|---|
| CPU recompilada vs. intérprete de PPSSPP | `./build/linux/cpu_diff` (todas las palabras de instrucción de BOOT.BIN) |
| Arranque sin ventana | `cd build/run && ../linux/ca_psprun --iso ../../game/SyphonFilterDarkMirror.iso --seconds 40` |
| Capturas, entrada guionizada | `ca_psprun --control FIFO` (press/hold/stick/shot…) |
| Descifrado real del ejecutable | solo verificable con un disco cuyo BOOT.BIN no esté en claro |

## Regresión respecto al original
Referencia: el mismo disco en PPSSPP. Por cada hito jugable se guardan (fuera del repositorio,
en `analysis/`) capturas de `ca_psprun shot` en puntos fijos de una secuencia de entrada
reproducible y se comparan con PPSSPP en los mismos puntos.

## Ejecutado en esta sesión (2026-10-30)
Linux, GCC 13, SDL2 2.30.9 compilado desde fuente: 4/4 pruebas superadas. Configuración con una
ISO sintética (BOOT.BIN vacío + EBOOT.BIN en claro): detecta `UCUS98641`, compila `sf_decrypt`
del host, extrae el módulo y lo pasa al recompilador (que falla con el PRX de juguete de las
pruebas, como se espera: no es un ejecutable real).
