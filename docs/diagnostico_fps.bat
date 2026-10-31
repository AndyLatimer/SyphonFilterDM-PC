@echo off
rem Diagnostico de fps: arranca el juego registrando, cada segundo, que esperas del sistema hace
rem y desde donde (sceDisplayWaitVblank, sceGeDrawSync, semaforos...). Juega 1 minuto dentro de
rem una mision, cierra el juego y envia SyphonFilterDarkMirror.log.
set CA_HLE_STATS=1
set CA_HLE_ONLY=Wait
SyphonFilterDarkMirror.exe
