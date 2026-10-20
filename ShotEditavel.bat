@echo off
rem Capturas automaticas (vistas fixas) no mapa editavel. Resultado em AutoShots\shot_N.png
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
echo INICIO %time% > "%~dp0ShotEditavel.log"
"%UE%" "%PRJ%" /Game/Maps/Fase1_Editavel?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding -AutoShot
echo SHOT_DONE %time% >> "%~dp0ShotEditavel.log"
