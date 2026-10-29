@echo off
rem Teste automatico da Fase 2 (capturas + log). Saida: AutoShots\fase2\
set "UE=C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe"
set "PRJ=%~dp0JogoCoop.uproject"
"%UE%" "%PRJ%" /Game/Maps/Fase2_Blockout?listen -game -windowed -ResX=1280 -ResY=720 -log -NoLiveCoding -Fase2Shot
echo SHOT_DONE %time% >> "%~dp0AutoShots\fase2\done.txt"
